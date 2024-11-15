#include "copy_effects.h"
#include "function/render/renderer_rd/storage/material_storage.h"
using namespace lain;
using namespace lain::RendererRD;
void CopyEffects::copy_to_rect(RID p_source_rd_texture, RID p_dest_texture, const Rect2i &p_rect, bool p_flip_y, bool p_force_luminance, bool p_all_source, bool p_8_bit_dst, bool p_alpha_to_one) {
	UniformSetCacheRD *uniform_set_cache = UniformSetCacheRD::get_singleton();
	ERR_FAIL_NULL(uniform_set_cache);
	MaterialStorage *material_storage = MaterialStorage::get_singleton();
	ERR_FAIL_NULL(material_storage);

	memset(&copy.push_constant, 0, sizeof(CopyPushConstant));
	if (p_flip_y) {
		copy.push_constant.flags |= COPY_FLAG_FLIP_Y;
	}

	if (p_force_luminance) {
		copy.push_constant.flags |= COPY_FLAG_FORCE_LUMINANCE;
	}

	if (p_all_source) {
		copy.push_constant.flags |= COPY_FLAG_ALL_SOURCE;
	}

	if (p_alpha_to_one) {
		copy.push_constant.flags |= COPY_FLAG_ALPHA_TO_ONE;
	}

	copy.push_constant.section[0] = p_rect.position.x;
	copy.push_constant.section[1] = p_rect.position.y;
	copy.push_constant.section[2] = p_rect.size.width();
	copy.push_constant.section[3] = p_rect.size.height();
	copy.push_constant.target[0] = p_rect.position.x;
	copy.push_constant.target[1] = p_rect.position.y;

	// setup our uniforms
	RID default_sampler = material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED);

	RD::Uniform u_source_rd_texture(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 0, Vector<RID>({ default_sampler, p_source_rd_texture }));
	RD::Uniform u_dest_texture(RD::UNIFORM_TYPE_IMAGE, 0, p_dest_texture);

	CopyMode mode = p_8_bit_dst ? COPY_MODE_SIMPLY_COPY_8BIT : COPY_MODE_SIMPLY_COPY;
	RID shader = copy.shader.version_get_shader(copy.shader_version, mode);
	ERR_FAIL_COND(shader.is_null());

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, copy.pipelines[mode]);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set_cache->get_cache(shader, 0, u_source_rd_texture), 0);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set_cache->get_cache(shader, 3, u_dest_texture), 3);
	RD::get_singleton()->compute_list_set_push_constant(compute_list, &copy.push_constant, sizeof(CopyPushConstant));
	RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_rect.size.width(), p_rect.size.height(), 1);
	RD::get_singleton()->compute_list_end();
}