#pragma once
#ifndef CLASS_DB_H
#define CLASS_DB_H
// 1. resource type and solver º¯Êý
// 2. Base and dertived relation
#include "core/string/string_name.h"
#include "core/templates/hash_map.h"
#include "core/templates/list.h"

namespace lain {
	class ClassDB {
	static HashMap<StringName, StringName> resource_base_extensions;
	public:
	static void add_resource_base_extension(const StringName& p_extension, const StringName& p_class);
	static void get_resource_base_extensions(List<String>* p_extensions);
	static StringName get_parent_class(const StringName& p_class);
	static bool class_exists(const StringName& p_class);
	};

}
#endif