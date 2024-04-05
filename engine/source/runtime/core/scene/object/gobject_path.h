#pragma once
#ifndef NODE_PATH_H
#define NODE_PATH_H

#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
namespace lain {

class GObjectPath {
	struct Data {
		SafeRefCount refcount;
		Vector<StringName> path;
		Vector<StringName> subpath;
		StringName concatenated_path;
		StringName concatenated_subpath;
		bool absolute;
		mutable bool hash_cache_valid;
		mutable uint32_t hash_cache;
	};

	mutable Data* data = nullptr;
	void unref();

	void _update_hash_cache() const;

public:
	bool is_absolute() const;
	int get_name_count() const;
	StringName get_name(int p_idx) const;
	int get_subname_count() const;
	StringName get_subname(int p_idx) const;
	Vector<StringName> get_names() const;
	Vector<StringName> get_subnames() const;
	StringName get_concatenated_names() const;
	StringName get_concatenated_subnames() const;

	GObjectPath rel_path_to(const GObjectPath& p_np) const;
	GObjectPath get_as_property_path() const;

	void prepend_period();

	_FORCE_INLINE_ uint32_t hash() const {
		if (!data) {
			return 0;
		}
		if (!data->hash_cache_valid) {
			_update_hash_cache();
		}
		return data->hash_cache;
	}

	operator String() const;
	bool is_empty() const;

	bool operator==(const GObjectPath& p_path) const;
	bool operator!=(const GObjectPath& p_path) const;
	void operator=(const GObjectPath& p_path);

	void simplify();
	GObjectPath simplified() const;

	GObjectPath(const Vector<StringName>& p_path, bool p_absolute);
	GObjectPath(const Vector<StringName>& p_path, const Vector<StringName>& p_subpath, bool p_absolute);
	GObjectPath(const GObjectPath& p_path);
	GObjectPath(const String& p_path);
	GObjectPath() {}
	~GObjectPath();
};
}

#endif