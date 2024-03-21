#pragma once
#ifndef MARSHALLS_H
#define MARSHALLS_H
#include "core/math/math_defs.h"
#include "core/typedefs.h"
#ifdef REAL_T_IS_DOUBLE
typedef uint64_t uintr_t;
#else
typedef uint32_t uintr_t;
#endif

/**
 * Miscellaneous helpers for marshaling data types, and encoding
 * in an endian independent way
 */
namespace lain {

union MarshallFloat {
	uint32_t i; ///< int
	float f; ///< float
};

union MarshallDouble {
	uint64_t l; ///< long long
	double d; ///< double
};

// Behaves like one of the above, depending on compilation setting.
union MarshallReal {
	uintr_t i;
	real_t r;
};
Error decode_variant(Variant& r_variant, const uint8_t* p_buffer, int p_len, int* r_len = nullptr, bool p_allow_objects = false, int p_depth = 0) { return OK; }
Error encode_variant(const Variant& p_variant, uint8_t* r_buffer, int& r_len, bool p_full_objects = false, int p_depth = 0) { return OK; }
}

#endif
