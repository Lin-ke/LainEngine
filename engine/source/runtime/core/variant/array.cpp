#include "array.h"
#include "base.h"
#include "core/math/hashfuncs.h"
#include "core/variant/variant.h"
namespace lain {
	class ArrayPrivate {
	public:
		SafeRefCount refcount;
		Vector<Variant> array;
		Variant* read_only = nullptr; // If enabled, a pointer is used to a temporary value that is used to return read-only values.
		//ContainerTypeValidate typed;
	};
	uint32_t Array::recursive_hash(int recursion_count) const {
		if (recursion_count > MAX_RECURSION) {
			L_CORE_ERROR("Max recursion reached");
			return 0;
		}

		uint32_t h = hash_murmur3_one_32(Variant::ARRAY);

		recursion_count++;
		for (int i = 0; i < _p->array.size(); i++) {
			h = hash_murmur3_one_32(_p->array[i].recursive_hash(recursion_count), h);
		}
		return hash_fmix32(h);
	}
}