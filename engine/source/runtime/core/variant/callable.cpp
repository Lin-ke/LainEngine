#include "callable.h"
#include "core/math/hashfuncs.h"
namespace lain {
	u32 Callable::hash() const {
		if (is_custom()) {
			return custom->hash();
		}
		else {
			uint32_t hash = method.hash();
			hash = hash_murmur3_one_64(object, hash);
			return hash_fmix32(hash);
		}
	}
	Callable::~Callable() {
		if (is_custom()) {
			if (custom->ref_count.unref()) {
				memdelete(custom);
			}
		}
	}
}