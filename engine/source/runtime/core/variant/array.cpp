#include "array.h"
#include "base.h"
#include "core/math/hashfuncs.h"
#include "core/variant/variant.h"
#include "variant_internal.h"
#include "container_type_validate.h"
namespace lain {
	class ArrayPrivate {
	public:
		SafeRefCount refcount;
		Vector<Variant> array;
		Variant* read_only = nullptr; // If enabled, a pointer is used to a temporary value that is used to return read-only values.
		ContainerTypeValidate typed;
	};
	uint32_t Array::recursive_hash(int recursion_count) const {
		if (recursion_count > MAX_RECURSION) {
			L_CORE_ERROR("Max recursion reached");
			return 0;
		}

		uint32_t h = hash_murmur3_one_32(Variant::ARRAY);

		recursion_count++;
		for (int i = 0; i < _p->array.size(); i++) {
			h = hash_murmur3_one_32(_p->array[i].recursive_hash(recursion_count), h);
		}
		return hash_fmix32(h);
	}
	void Array::set(int p_idx, const Variant& p_value) {
		ERR_FAIL_COND_MSG(_p->read_only, "Array is in read-only state.");
		Variant value = p_value;
		//ERR_FAIL_COND(!_p->typed.validate(value, "set"));

		operator[](p_idx) = value;
	}

	const Variant& Array::get(int p_idx) const {
		return operator[](p_idx);
	}

	const Variant& Array::operator[](int p_idx) const {
		if (unlikely(_p->read_only)) {
			*_p->read_only = _p->array[p_idx];
			return *_p->read_only;
		}
		return _p->array[p_idx];
	}

	Variant& Array::operator[](int p_idx) {
		if (unlikely(_p->read_only)) {
			*_p->read_only = _p->array[p_idx];
			return *_p->read_only;
		}
		return _p->array.write[p_idx];
	}


	int Array::size() const {
		return _p->array.size();
	}

	bool Array::is_empty() const {
		return _p->array.is_empty();
	}

	Error Array::resize(int p_new_size) {
		ERR_FAIL_COND_V_MSG(_p->read_only, ERR_LOCKED, "Array is in read-only state.");
		Variant::Type& variant_type = _p->typed.type;
		int old_size = _p->array.size();
		Error err = _p->array.resize_zeroed(p_new_size);
		if (!err && variant_type != Variant::NIL && variant_type != Variant::OBJECT) {
			for (int i = old_size; i < p_new_size; i++) {
				VariantInternal::initialize(&_p->array.write[i], variant_type);
			}
		}
		return err;
	}

	Array::Array(const Array& p_from, uint32_t p_type, const StringName& p_class_name, const Variant& p_script) {
		_p = memnew(ArrayPrivate);
		_p->refcount.init();
		set_typed(p_type, p_class_name, p_script);
		assign(p_from);
	}

	Array::Array(const Array& p_from) {
		_p = nullptr;
		_ref(p_from);
	}

	Array::Array() {
		_p = memnew(ArrayPrivate);
		_p->refcount.init();
	}

	Array::~Array() {
		_unref();
	}

	void Array::_unref() const {
		if (!_p) {
			return;
		}

		if (_p->refcount.unref()) {
			if (_p->read_only) {
				memdelete(_p->read_only);
			}
			memdelete(_p);
		}
		_p = nullptr;
	}

	void Array::_ref(const Array& p_from) const {
		ArrayPrivate* _fp = p_from._p;

		ERR_FAIL_NULL(_fp); // Should NOT happen.

		if (_fp == _p) {
			return; // whatever it is, nothing to do here move along
		}

		bool success = _fp->refcount.ref();

		ERR_FAIL_COND(!success); // should really not happen either

		_unref();

		_p = _fp;
	}


	void Array::set_typed(uint32_t p_type, const StringName& p_class_name, const Variant& p_script) {
		ERR_FAIL_COND_MSG(_p->read_only, "Array is in read-only state.");
		ERR_FAIL_COND_MSG(_p->array.size() > 0, "Type can only be set when array is empty.");
		ERR_FAIL_COND_MSG(_p->refcount.get() > 1, "Type can only be set when array has no more than one user.");
		ERR_FAIL_COND_MSG(_p->typed.type != Variant::NIL, "Type can only be set once.");
		ERR_FAIL_COND_MSG(p_class_name != StringName() && p_type != Variant::OBJECT, "Class names can only be set for type OBJECT");
		//Ref<Script> script = p_script;
		//ERR_FAIL_COND_MSG(script.is_valid() && p_class_name == StringName(), "Script class can only be set together with base class name");

		_p->typed.type = Variant::Type(p_type);
		_p->typed.class_name = p_class_name;
		//_p->typed.script = script;
		_p->typed.where = "TypedArray";
	}

	void Array::assign(const Array& p_array) {
		const ContainerTypeValidate& typed = _p->typed;
		const ContainerTypeValidate& source_typed = p_array._p->typed;

		if (typed == source_typed || typed.type == Variant::NIL || (source_typed.type == Variant::OBJECT && typed.can_reference(source_typed))) {
			// from same to same or
			// from anything to variants or
			// from subclasses to base classes
			_p->array = p_array._p->array;
			return;
		}

		const Variant* source = p_array._p->array.ptr();
		int size = p_array._p->array.size();

		if ((source_typed.type == Variant::NIL && typed.type == Variant::OBJECT) || (source_typed.type == Variant::OBJECT && source_typed.can_reference(typed))) {
			// from variants to objects or
			// from base classes to subclasses
			for (int i = 0; i < size; i++) {
				const Variant& element = source[i];
				if (element.get_type() != Variant::NIL && (element.get_type() != Variant::OBJECT || !typed.validate_object(element, "assign"))) {
					ERR_FAIL_MSG(vformat(R"(Unable to convert array index %i from "%s" to "%s".)", i, Variant::get_type_name(element.get_type()), Variant::get_type_name(typed.type)));
				}
			}
			_p->array = p_array._p->array;
			return;
		}
		if (typed.type == Variant::OBJECT || source_typed.type == Variant::OBJECT) {
			ERR_FAIL_MSG(vformat(R"(Cannot assign contents of "Array[%s]" to "Array[%s]".)", Variant::get_type_name(source_typed.type), Variant::get_type_name(typed.type)));
		}

		Vector<Variant> array;
		array.resize(size);
		Variant* data = array.ptrw();

		if (source_typed.type == Variant::NIL && typed.type != Variant::OBJECT) {
			// from variants to primitives
			for (int i = 0; i < size; i++) {
				const Variant* value = source + i;
				if (value->get_type() == typed.type) {
					data[i] = *value;
					continue;
				}
				if (!Variant::can_convert_strict(value->get_type(), typed.type)) {
					ERR_FAIL_MSG("Unable to convert array index " + itos(i) + " from '" + Variant::get_type_name(value->get_type()) + "' to '" + Variant::get_type_name(typed.type) + "'.");
				}
				//Callable::CallError ce;
				//Variant::construct(typed.type, data[i], &value, 1, ce);
				//ERR_FAIL_COND_MSG(ce.error, vformat(R"(Unable to convert array index %i from "%s" to "%s".)", i, Variant::get_type_name(value->get_type()), Variant::get_type_name(typed.type)));
			}
		}
		else if (Variant::can_convert_strict(source_typed.type, typed.type)) {
			// from primitives to different convertible primitives
			for (int i = 0; i < size; i++) {
				const Variant* value = source + i;
				//Callable::CallError ce;
				//Variant::construct(typed.type, data[i], &value, 1, ce);
				//ERR_FAIL_COND_MSG(ce.error, vformat(R"(Unable to convert array index %i from "%s" to "%s".)", i, Variant::get_type_name(value->get_type()), Variant::get_type_name(typed.type)));
			}
		}
		else {
			ERR_FAIL_MSG(vformat(R"(Cannot assign contents of "Array[%s]" to "Array[%s]".)", Variant::get_type_name(source_typed.type), Variant::get_type_name(typed.type)));
		}

		_p->array = array;
	}

}