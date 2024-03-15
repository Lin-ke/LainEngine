#pragma once
#ifndef RESOURCE_UID_H
#define RESOURCE_UID_H

#include "core/object/refcounted.h"
#include "core/string/string_name.h"
#include "core/templates/hash_map.h"
namespace lain {

class ResourceUID : public Object {
public:
	typedef int64_t ID;
	enum {
		INVALID_ID = -1
	};

	static String get_cache_file();

private:
	void* crypto = nullptr; // CryptoCore::RandomGenerator (avoid including crypto_core.h)
	Mutex mutex;
	struct Cache {
		CharString cs;
		bool saved_to_cache = false;
	};

	HashMap<ID, Cache> unique_ids; //unique IDs and utf8 paths (less memory used)
	static ResourceUID* singleton;

	uint32_t cache_entries = 0;
	bool changed = false;

protected:
	static void _bind_methods();

public:
	String id_to_text(ID p_id) const;
	ID text_to_id(const String& p_text) const;

	ID create_id();
	bool has_id(ID p_id) const;
	void add_id(ID p_id, const String& p_path);
	void set_id(ID p_id, const String& p_path);
	String get_id_path(ID p_id) const;
	void remove_id(ID p_id);

	Error load_from_cache();
	Error save_to_cache();
	Error update_cache();

	void clear();

	static ResourceUID* GetSingleton() { return singleton; }

	ResourceUID();
	~ResourceUID();
};

}
#endif // RESOURCE_UID_H
