#include "core/config/project_settings.h"
#include "core/os/os.h"
#include "file_access.h"
#include <filesystem>
#include "marshalls.h"

namespace lain {
	FileAccess::CreateFunc FileAccess::create_func[ACCESS_MAX] = { nullptr, nullptr, nullptr };
	FileAccess::FileCloseFailNotify FileAccess::close_fail_notify = nullptr;

	bool FileAccess::backup_save = false;
	thread_local Error FileAccess::last_file_open_error = OK;

	class CharBuffer {
		Vector<char> vector;
		char stack_buffer[256];

		char* buffer = nullptr;
		int capacity = 0;
		int written = 0;

		bool grow() {
			if (vector.resize(next_power_of_2(1 + written)) != OK) {
				return false;
			}

			if (buffer == stack_buffer) { // first chunk?

				for (int i = 0; i < written; i++) {
					vector.write[i] = stack_buffer[i];
				}
			}

			buffer = vector.ptrw();
			capacity = vector.size();
			ERR_FAIL_COND_V(written >= capacity, false);

			return true;
		}

	public:
		_FORCE_INLINE_ CharBuffer() :
			buffer(stack_buffer),
			capacity(sizeof(stack_buffer) / sizeof(char)) {
		}

		_FORCE_INLINE_ void push_back(char c) {
			if (written >= capacity) {
				ERR_FAIL_COND(!grow());
			}

			buffer[written++] = c;
		}

		_FORCE_INLINE_ const char* get_data() const {
			return buffer;
		}
	};

	Ref<FileAccess> FileAccess::open(const String& p_path, int p_mode_flags, Error* r_error) {
		//try packed data first

		Ref<FileAccess> ret;
		/*if (!(p_mode_flags & WRITE) && PackedData::GetSingleton() && !PackedData::GetSingleton()->is_disabled()) {
			ret = PackedData::GetSingleton()->try_open_path(p_path);
			if (ret.is_valid()) {
				if (r_error) {
					*r_error = OK;
				}
				return ret;
			}
		}*/

		ret = create_for_path(p_path);
		Error err = ret->open_internal(p_path, p_mode_flags);

		if (r_error) {
			*r_error = err;
		}
		if (err != OK) {
			ret.unref();
		}

		return ret;
	}
	Ref<FileAccess> FileAccess::create(AccessType p_access) {
		ERR_FAIL_INDEX_V(p_access, ACCESS_MAX, nullptr);
		// operator=
		Ref<FileAccess> ret = create_func[p_access]();
		ret->_set_access_type(p_access);
		return ret;
	}

	Ref<FileAccess> FileAccess::create_for_path(const String& p_path) {
		Ref<FileAccess> ret;
		if (p_path.begins_with("res://")) {
			ret = create(ACCESS_RESOURCES);
		}
		else if (p_path.begins_with("user://")) {
			ret = create(ACCESS_USERDATA);

		}
		else {
			ret = create(ACCESS_FILESYSTEM);
		}

		return ret;
	}


	FileAccess::CreateFunc FileAccess::get_create_func(AccessType p_access) {
		return create_func[p_access];
	}
	void FileAccess::_set_access_type(AccessType p_access) {
		_access_type = p_access;
	}

	FileAccess::AccessType FileAccess::get_access_type() const {
		return _access_type;
	}

	/// <summary>
	/// turn relative path(res:// to real urls.)
	/// </summary>
	/// <param name="p_path"></param>
	/// <returns></returns>
	String FileAccess::fix_path(const String& p_path) const {
		//helper used by file accesses that use a single filesystem

		String r_path = p_path.replace("\\", "/");

		switch (_access_type) {
		case ACCESS_RESOURCES: {
			if (ProjectSettings::GetSingleton()) {
				if (r_path.begins_with("res://")) {
					String resource_path = ProjectSettings::GetSingleton()->GetResourcePath();
					if (!resource_path.is_empty()) {
						return r_path.replace("res:/", resource_path);
					}
					return r_path.replace("res://", "");
				}
			}

		} break;
		case ACCESS_USERDATA: {
			if (r_path.begins_with("user://")) {
				String data_dir = OS::GetSingleton()->GetUserDataDir();
				if (!data_dir.is_empty()) {
					return r_path.replace("user:/", data_dir);
				}
				return r_path.replace("user://", "");
			}

		} break;
		case ACCESS_FILESYSTEM: {
			return r_path;
		} break;
		case ACCESS_MAX:
			break; // Can't happen, but silences warning
		}

		return r_path;
	}

	bool FileAccess::exists(const String& p_name) {
		/*if (PackedData::GetSingleton() && !PackedData::GetSingleton()->is_disabled() && PackedData::GetSingleton()->has_path(p_name)) {
			return true;
		}*/
		Ref<FileAccess> f = open(p_name, READ);
		if (f.is_null()) {
			return false;
		}
		return true;
	}
	uint64_t FileAccess::get_modified_time(const String& p_file) {
		Ref<FileAccess> fa = create_for_path(p_file);
		ERR_FAIL_COND_V_MSG(fa.is_null(), 0, "Cannot create FileAccess for path '" + p_file + "'.");

		uint64_t mt = fa->_get_modified_time(p_file);
		return mt;
	}

	Vector<uint8_t> FileAccess::get_file_as_bytes(const String& p_path, Error* r_error) {
		Ref<FileAccess> f = FileAccess::open(p_path, READ, r_error);
		if (f.is_null()) {
			if (r_error) { // if error requested, do not throw error
				return Vector<uint8_t>();
			}
			ERR_FAIL_V_MSG(Vector<uint8_t>(), "Can't open file from path '" + String(p_path) + "'.");
		}
		Vector<uint8_t> data;
		data.resize(f->get_length());
		f->get_buffer(data.ptrw(), data.size());
		return data;
	}
	String FileAccess::get_file_as_string(const String& p_path, Error* r_error) {
		Error err;
		Vector<uint8_t> array = get_file_as_bytes(p_path, &err);
		if (r_error) {
			*r_error = err;
		}
		if (err != OK) {
			if (r_error) {
				return String();
			}
			ERR_FAIL_V_MSG(String(), "Can't get file as string from path '" + String(p_path) + "'.");
		}

		String ret;
		ret.parse_utf8((const char*)array.ptr(), array.size());
		return ret;
	}
	PackedByteArray FileAccess::_get_file_as_bytes(const String& p_path) { return get_file_as_bytes(p_path, &last_file_open_error); }
	String  FileAccess::_get_file_as_string(const String& p_path) { return get_file_as_string(p_path, &last_file_open_error); }

	uint64_t FileAccess::get_buffer(uint8_t* p_dst, uint64_t p_length) const {
		ERR_FAIL_COND_V(!p_dst && p_length > 0, -1);

		uint64_t i = 0;
		for (i = 0; i < p_length && !eof_reached(); i++) {
			p_dst[i] = get_8();
		}

		return i;
	}

	Vector<uint8_t> FileAccess::get_buffer(int64_t p_length) const {
		Vector<uint8_t> data;

		ERR_FAIL_COND_V_MSG(p_length < 0, data, "Length of buffer cannot be smaller than 0.");
		if (p_length == 0) {
			return data;
		}

		Error err = data.resize(p_length);
		ERR_FAIL_COND_V_MSG(err != OK, data, "Can't resize data to " + itos(p_length) + " elements.");

		uint8_t* w = data.ptrw();
		int64_t len = get_buffer(&w[0], p_length);

		if (len < p_length) {
			data.resize(len);
		}

		return data;
	}
	// 处理的情况是假设文件是UTF8格式的。

	String FileAccess::get_line() const {
		// 类似stringbuffer
		CharBuffer line;
		char32_t c = get_8();

		while (!eof_reached()) {
			if (c == '\n' || c == '\0') {
				line.push_back(0);
				return String::utf8(line.get_data());
			}
			else if (c != '\r') {
				line.push_back(c);
			}

			c = get_8();
		}
		line.push_back(0);
		return String::utf8(line.get_data());
	}

	String FileAccess::get_token() const {
		CharString token;
		char32_t c = get_8();

		while (!eof_reached()) {
			if (c <= ' ') {
				if (token.length()) {
					break;
				}
			}
			else {
				token += c;
			}
			c = get_8();
		}

		return String::utf8(token.get_data());
	}

	Vector<String> FileAccess::get_csv_line(const String& p_delim) const {
		ERR_FAIL_COND_V_MSG(p_delim.length() != 1, Vector<String>(), "Only single character delimiters are supported to parse CSV lines.");
		ERR_FAIL_COND_V_MSG(p_delim[0] == '"', Vector<String>(), "The double quotation mark character (\") is not supported as a delimiter for CSV lines.");

		String line;

		// CSV can support entries with line breaks as long as they are enclosed
		// in double quotes. So our "line" might be more than a single line in the
		// text file.
		int qc = 0;
		do {
			if (eof_reached()) {
				break;
			}
			line += get_line() + "\n";
			qc = 0;
			for (int i = 0; i < line.length(); i++) {
				if (line[i] == '"') {
					qc++;
				}
			}
		} while (qc % 2);

		// Remove the extraneous newline we've added above.
		line = line.substr(0, line.length() - 1);

		Vector<String> strings;

		bool in_quote = false;
		String current;
		for (int i = 0; i < line.length(); i++) {
			char32_t c = line[i];
			// A delimiter ends the current entry, unless it's in a quoted string.
			if (!in_quote && c == p_delim[0]) {
				strings.push_back(current);
				current = String();
			}
			else if (c == '"') {
				// Doubled quotes are escapes for intentional quotes in the string.
				if (line[i + 1] == '"' && in_quote) {
					current += '"';
					i++;
				}
				else {
					in_quote = !in_quote;
				}
			}
			else {
				current += c;
			}
		}

		if (in_quote) {
			WARN_PRINT(vformat("Reached end of file before closing '\"' in CSV file '%s'.", get_path()));
		}

		strings.push_back(current);

		return strings;
	}
	/* these are all implemented for ease of porting, then can later be optimized */
	// 这里和store一样都是重复代码

	uint16_t FileAccess::get_16() const {
		uint16_t res;
		uint8_t a, b;

		a = get_8();
		b = get_8();

		if (big_endian) {
			SWAP(a, b);
		}

		res = b;
		res <<= 8;
		res |= a;

		return res;
	}

	uint32_t FileAccess::get_32() const {
		uint32_t res;
		uint16_t a, b;

		a = get_16();
		b = get_16();

		if (big_endian) {
			SWAP(a, b);
		}

		res = b;
		res <<= 16;
		res |= a;

		return res;
	}


	uint64_t FileAccess::get_64() const {
		uint64_t res;
		uint32_t a, b;

		a = get_32();
		b = get_32();

		if (big_endian) {
			SWAP(a, b);
		}

		res = b;
		res <<= 32;
		res |= a;

		return res;
	}


	float FileAccess::get_float() const {
		MarshallFloat m;
		m.i = get_32();
		return m.f;
	}
	double FileAccess::get_double() const {
		MarshallDouble m;
		m.l = get_64();
		return m.d;
	}

	real_t FileAccess::get_real() const {
		if (real_is_double) {
			return get_double();
		}
		else {
			return get_float();
		}
	}

	Variant FileAccess::get_var(bool p_allow_objects) const {
		uint32_t len = get_32();
		Vector<uint8_t> buff = get_buffer(len);
		ERR_FAIL_COND_V((uint32_t)buff.size() != len, Variant());

		const uint8_t* r = buff.ptr();

		Variant v;
		Error err = decode_variant(v, &r[0], len, nullptr, p_allow_objects);
		ERR_FAIL_COND_V_MSG(err != OK, Variant(), "Error when trying to encode Variant.");

		return v;
	}

	void FileAccess::store_16(uint16_t p_dest) {
		uint8_t a, b;

		a = p_dest & 0xFF;
		b = p_dest >> 8;

		if (big_endian) {
			SWAP(a, b);
		}

		store_8(a);
		store_8(b);
	}

	void FileAccess::store_32(uint32_t p_dest) {
		uint16_t a, b;

		a = p_dest & 0xFFFF;
		b = p_dest >> 16;

		if (big_endian) {
			SWAP(a, b);
		}

		store_16(a);
		store_16(b);
	}

	void FileAccess::store_64(uint64_t p_dest) {
		uint32_t a, b;

		a = p_dest & 0xFFFFFFFF;
		b = p_dest >> 32;

		if (big_endian) {
			SWAP(a, b);
		}

		store_32(a);
		store_32(b);
	}

	void FileAccess::store_real(real_t p_real) {
		if constexpr (sizeof(real_t) == 4) {
			store_float(p_real);
		}
		else {
			store_double(p_real);
		}
	}
	// 通过union存储字节，以防止存储时强转
	void FileAccess::store_float(float p_dest) {
		MarshallFloat m;
		m.f = p_dest;
		store_32(m.i);
	}

	void FileAccess::store_double(double p_dest) {
		MarshallDouble m;
		m.d = p_dest;
		store_64(m.l);
	}
	void FileAccess::store_string(const String& p_string) {
		if (p_string.length() == 0) {
			return;
		}

		CharString cs = p_string.utf8();
		store_buffer((uint8_t*)&cs[0], cs.length());
	}
	void FileAccess::store_line(const String& p_line) {
		store_string(p_line);
		store_8('\n');
	}
	Error FileAccess::reopen(const String& p_path, int p_mode_flags) {
		return open_internal(p_path, p_mode_flags);
	}
	void FileAccess::store_csv_line(const Vector<String>& p_values, const String& p_delim) {
		ERR_FAIL_COND(p_delim.length() != 1);

		String line = "";
		int size = p_values.size();
		for (int i = 0; i < size; ++i) {
			String value = p_values[i];

			if (value.contains("\"") || value.contains(p_delim) || value.contains("\n")) {
				value = "\"" + value.replace("\"", "\"\"") + "\"";
			}
			if (i < size - 1) {
				value += p_delim;
			}

			line += value;
		}

		store_line(line);
	}
	void FileAccess::store_pascal_string(const String& p_string) {
		CharString cs = p_string.utf8();
		store_32(cs.length());
		store_buffer((uint8_t*)&cs[0], cs.length());
	}
	String FileAccess::get_pascal_string() {
		uint32_t sl = get_32();
		CharString cs;
		cs.resize(sl + 1);
		get_buffer((uint8_t*)cs.ptr(), sl);
		cs[sl] = 0;

		String ret;
		ret.parse_utf8(cs.ptr());
		return ret;
	}
	void FileAccess::store_buffer(const uint8_t* p_src, uint64_t p_length) {
		ERR_FAIL_COND(!p_src && p_length > 0);
		for (uint64_t i = 0; i < p_length; i++) {
			store_8(p_src[i]);
		}
	}
	void FileAccess::store_buffer(const Vector<uint8_t>& p_buffer) {
		uint64_t len = p_buffer.size();
		if (len == 0) {
			return;
		}

		const uint8_t* r = p_buffer.ptr();

		store_buffer(&r[0], len);
	}

	String FileAccess::get_as_text(bool p_skip_cr) const {
		uint64_t original_pos = get_position();
		const_cast<FileAccess*>(this)->seek(0);

		String text = get_as_utf8_string(p_skip_cr);

		const_cast<FileAccess*>(this)->seek(original_pos);

		return text;
	}

	String FileAccess::get_residual_text(bool p_skip_cr) const {
		Vector<uint8_t> sourcef;
		uint64_t len = get_length() - get_position();
		sourcef.resize(len + 1);

		uint8_t* w = sourcef.ptrw();
		uint64_t r = get_buffer(w, len);
		ERR_FAIL_COND_V(r != len, String());
		w[len] = 0;

		String s;
		s.parse_utf8((const char*)w, -1, p_skip_cr);
		return s;
	}



	String FileAccess::get_as_utf8_string(bool p_skip_cr) const {
		Vector<uint8_t> sourcef;
		uint64_t len = get_length();
		sourcef.resize(len + 1);

		uint8_t* w = sourcef.ptrw();
		uint64_t r = get_buffer(w, len);
		ERR_FAIL_COND_V(r != len, String());
		w[len] = 0;

		String s;
		s.parse_utf8((const char*)w, -1, p_skip_cr);
		return s;
	}
	/// TODO:
	void FileAccess::store_var(const Variant& p_var, bool p_full_objects) {
		/*int len;
		Error err = encode_variant(p_var, nullptr, len, p_full_objects);
		ERR_FAIL_COND_MSG(err != OK, "Error when trying to encode Variant.");

		Vector<uint8_t> buff;
		buff.resize(len);

		uint8_t* w = buff.ptrw();
		err = encode_variant(p_var, &w[0], len, p_full_objects);
		ERR_FAIL_COND_MSG(err != OK, "Error when trying to encode Variant.");

		store_32(len);
		store_buffer(buff);*/
	}
}