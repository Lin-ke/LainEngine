#pragma once
#ifndef __PAIR_H__
#define __PAIR_H__

#include "base.h"
//#include "tuple.h"
#include <tuple>
#define Tuple std::tuple
template<int32_t Idx, typename... Elem>
auto __tuple_get(const Tuple<Elem...>& tuple) {
	return std::get<Idx>(tuple);
}
namespace lain {

template <class F, class S>
struct Pair {
	typedef F ftype;
	typedef S stype;

	F first;
	S second;

	Pair() :
		first(),
		second() {
	}

	Pair(F p_first, const S& p_second) :
		first(p_first),
		second(p_second) {
	}
};

template <class F, class S>
bool operator==(const Pair<F, S>& pair, const Pair<F, S>& other) {
	return (pair.first == other.first) && (pair.second == other.second);
}

template <class F, class S>
bool operator!=(const Pair<F, S>& pair, const Pair<F, S>& other) {
	return (pair.first != other.first) || (pair.second != other.second);
}

template <class F, class S>
struct PairSort {
	bool operator()(const Pair<F, S>& A, const Pair<F, S>& B) const {
		if (A.first != B.first) {
			return A.first < B.first;
		}
		return A.second < B.second;
	}
};

template <class F, class S>
struct PairHash {
	static uint32_t hash(const Pair<F, S>& P) {
		uint64_t h1 = HashMapHasherDefault::hash(P.first);
		uint64_t h2 = HashMapHasherDefault::hash(P.second);
		return hash_one_uint64((h1 << 32) | h2);
	}
};

template <class K, class V>
struct KeyValue {
	const K key;
	V value;

	void operator=(const KeyValue& p_kv) = delete;
	_FORCE_INLINE_ KeyValue(const KeyValue& p_kv) :
		key(p_kv.key),
		value(p_kv.value) {
	}
	_FORCE_INLINE_ KeyValue(const K& p_key, const V& p_value) :
		key(p_key),
		value(p_value) {
	}
};

template <class K, class V>
bool operator==(const KeyValue<K, V>& pair, const KeyValue<K, V>& other) {
	return (pair.key == other.key) && (pair.value == other.value);
}

template <class K, class V>
bool operator!=(const KeyValue<K, V>& pair, const KeyValue<K, V>& other) {
	return (pair.key != other.key) || (pair.value != other.value);
}

template <class K, class V>
struct KeyValueSort {
	bool operator()(const KeyValue<K, V>& A, const KeyValue<K, V>& B) const {
		return A.key < B.key;
	}
};

template <class K, class... V>
struct KeyValues {
	const K key;
	Tuple<V...> value;

	void operator=(const KeyValues& p_kv) = delete;
	KeyValues(const KeyValues& p_kv) :
		key(p_kv.key),
		value(p_kv.value)
	{
	}

	template <typename K, class... V>
	KeyValues(const K& p_key, const V&... p_value) :
		key(p_key),
	{
	 values = make_tuple(p_value...);
	}
};

}

#endif // !__PAIRS_H__
