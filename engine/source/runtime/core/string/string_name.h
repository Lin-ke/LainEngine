#pragma once
#ifndef __STRING_NAME_H__
#define __STRING_NAME_H__
#include "base.h"
#include "core/templates/safe_numeric.h"
#include "core/object/safe_refcount.h"

//https://docs.godotengine.org/en/stable/classes/class_stringname.html
// For compare speed
class StringName {
	struct _Data
	{
		SafeRefCount refcount;
		SafeNumeric<uint32_t> static_count;
		const char* cname = nullptr;
		String name;
#ifdef L_DEBUG
		uint32_t debug_references = 0;
#endif
		String get_name() const { return cname ? String(cname) : name; }
		int idx = 0;
		uint32_t hash = 0;
		_Data* prev = nullptr;
		_Data* next = nullptr;
		_Data() {}
	};
};
#endif // !__STRING_NAME_H__
