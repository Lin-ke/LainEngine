#pragma once
#ifndef __META_ENUM_H__
#define __META_ENUM_H__
#include "core/string/ustring.h"
#include "core/string/string_name.h"
#include "core/templates/hash_map.h"
namespace lain {
	class EnumDB {
		static HashMap<StringName, HashMap<int, StringName>> enums;
		static int string_to_enum(const String& p_str) {

		}
		static String enum_to_string(const int p_enum) {

		}
	};

}
#endif