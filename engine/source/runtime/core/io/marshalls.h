#pragma once
#ifndef MARSHALLS_H
#define MARSHALLS_H
#include "core/math/math_defs.h"
#include "core/typedefs.h"
#include "core/variant/variant.h"
#include "core/object/refcounted.h"
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
// 将32位整数编码到字节数组中
static inline unsigned int encode_uint32(uint32_t p_uint, uint8_t *p_arr) {
	for (int i = 0; i < 4; i++) {
		*p_arr = p_uint & 0xFF;
		p_arr++;
		p_uint >>= 8;
	}

	return sizeof(uint32_t);
}
static inline uint32_t decode_uint32(const uint8_t *p_arr) {
	uint32_t u = 0;

	for (int i = 0; i < 4; i++) {
		uint32_t b = *p_arr;
		b <<= (i * 8);
		u |= b;
		p_arr++;
	}

	return u;
}

static inline unsigned int encode_uint64(uint64_t p_uint, uint8_t *p_arr) {
	for (int i = 0; i < 8; i++) {
		*p_arr = p_uint & 0xFF;
		p_arr++;
		p_uint >>= 8;
	}

	return sizeof(uint64_t);
}


static inline uint64_t decode_uint64(const uint8_t *p_arr) {
	uint64_t u = 0;

	for (int i = 0; i < 8; i++) {
		uint64_t b = (*p_arr) & 0xFF;
		b <<= (i * 8);
		u |= b;
		p_arr++;
	}

	return u;
}
static inline unsigned int encode_double(double p_double, uint8_t *p_arr) {
	MarshallDouble md;
	md.d = p_double;
	encode_uint64(md.l, p_arr);

	return sizeof(uint64_t);
}

static inline double decode_double(const uint8_t *p_arr) {
	MarshallDouble md;
	md.l = decode_uint64(p_arr);
	return md.d;
}
static inline unsigned int encode_float(float p_float, uint8_t *p_arr) {
	MarshallFloat mf;
	mf.f = p_float;
	encode_uint32(mf.i, p_arr);

	return sizeof(uint32_t);
}
static inline float decode_float(const uint8_t *p_arr) {
	MarshallFloat mf;
	mf.i = decode_uint32(p_arr);
	return mf.f;
}
Error decode_variant(Variant& r_variant, const uint8_t* p_buffer, int p_len, int* r_len = nullptr, bool p_allow_objects = false, int p_depth = 0);
Error encode_variant(const Variant& p_variant, uint8_t* r_buffer, int& r_len, bool p_full_objects = false, int p_depth = 0);
class EncodedObjectAsID : public RefCounted {
	LCLASS(EncodedObjectAsID, RefCounted);

	ObjectID id;

protected:
	// static void _bind_methods();

public:
	void set_object_id(ObjectID p_id);
	ObjectID get_object_id() const;

	EncodedObjectAsID() {}
}; 

}

#endif
