/**************************************************************************/
/*  uniform_set_cache_rd.cpp                                              */
/**************************************************************************/

#include "uniform_set_cache_rd.h"
using namespace lain;
UniformSetCacheRD *UniformSetCacheRD::singleton = nullptr;

void UniformSetCacheRD::_bind_methods() {
	// ClassDB::bind_static_method("UniformSetCacheRD", D_METHOD("get_cache", "shader", "set", "uniforms"), &UniformSetCacheRD::get_cache_array);
}

RID UniformSetCacheRD::get_cache_array(RID p_shader, uint32_t p_set, const Vector<Ref<RDUniform>> &p_uniforms) {
	Vector<RD::Uniform> uniforms;

	for (int i = 0; i < p_uniforms.size(); i++) {
		Ref<RDUniform> uniform = p_uniforms[i];
		if (uniform.is_valid()) {
			uniforms.push_back(uniform->base);
		}
	}

	return UniformSetCacheRD::get_singleton()->get_cache_vec(p_shader, p_set, uniforms);
}

void UniformSetCacheRD::_invalidate(Cache *p_cache) {
	if (p_cache->prev) {
		p_cache->prev->next = p_cache->next;
	} else {
		// At beginning of table
		uint32_t table_idx = p_cache->hash % HASH_TABLE_SIZE;
		hash_table[table_idx] = p_cache->next;
	}

	if (p_cache->next) {
		p_cache->next->prev = p_cache->prev;
	}

	cache_allocator.free(p_cache);
	cache_instances_used--;
}
void UniformSetCacheRD::_uniform_set_invalidation_callback(void *p_userdata) {
	singleton->_invalidate(reinterpret_cast<Cache *>(p_userdata));
}

UniformSetCacheRD::UniformSetCacheRD() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

UniformSetCacheRD::~UniformSetCacheRD() {
	if (cache_instances_used > 0) {
		ERR_PRINT("At exit: " + itos(cache_instances_used) + " uniform set cache instance(s) still in use.");
	}
}
