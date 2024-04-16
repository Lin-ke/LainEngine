#pragma once

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "core/string/ustring.h"
#include "core/templates/list.h"
#include "core/variant/array.h"
#include "core/meta/reflection/reflection.h"
namespace lain {

class Variant;

struct DictionaryPrivate;
class Dictionary{
	mutable DictionaryPrivate* _p;

	void _ref(const Dictionary& p_from) const;
	void _unref() const;

public:
	void get_key_list(List<Variant>* p_keys) const;
	Variant get_key_at_index(int p_index) const;
	Variant get_value_at_index(int p_index) const;

	Variant& operator[](const Variant& p_key);
	const Variant& operator[](const Variant& p_key) const;

	const Variant* getptr(const Variant& p_key) const;
	Variant* getptr(const Variant& p_key);

	Variant get_valid(const Variant& p_key) const;
	Variant get(const Variant& p_key, const Variant& p_default) const;

	int size() const;
	bool is_empty() const;
	void clear();
	void merge(const Dictionary& p_dictionary, bool p_overwrite = false);

	bool has(const Variant& p_key) const;
	bool has_all(const Array& p_keys) const;
	Variant find_key(const Variant& p_value) const;

	bool erase(const Variant& p_key);

	bool operator==(const Dictionary& p_dictionary) const;
	bool operator!=(const Dictionary& p_dictionary) const;
	bool recursive_equal(const Dictionary& p_dictionary, int recursion_count) const;

	uint32_t hash() const;
	uint32_t recursive_hash(int recursion_count) const;
	void operator=(const Dictionary& p_dictionary);

	const Variant* next(const Variant* p_key = nullptr) const;

	Array keys() const;
	Array values() const;

	Dictionary duplicate(bool p_deep = false) const;
	Dictionary recursive_duplicate(bool p_deep, int recursion_count) const;

	void make_read_only();
	bool is_read_only() const;

	const void* id() const;

	Dictionary(const Dictionary& p_from);
	Dictionary();
	~Dictionary();
};
}

#endif // DICTIONARY_H
