/**************************************************************************/
/*  resolve.cpp                                                           */
/**************************************************************************/

#include "resolve.h"
#include "function/render/renderer_rd/renderer_compositor_rd.h"
#include "function/render/renderer_rd/storage/material_storage.h"
#include "function/render/renderer_rd/uniform_set_cache_rd.h"

using namespace lain::RendererRD;

Resolve::Resolve() {
	Vector<String> resolve_modes;
	resolve_modes.push_back("\n#define MODE_RESOLVE_GI\n");
	resolve_modes.push_back("\n#define MODE_RESOLVE_GI\n#define VOXEL_GI_RESOLVE\n");
	resolve_modes.push_back("\n#define MODE_RESOLVE_DEPTH\n");

	resolve.shader.initialize(resolve_modes);

	resolve.shader_version = resolve.shader.version_create();

	for (int i = 0; i < RESOLVE_MODE_MAX; i++) {
		resolve.pipelines[i] = RD::get_singleton()->compute_pipeline_create(resolve.shader.version_get_shader(resolve.shader_version, i));
	}
}

Resolve::~Resolve() {
	resolve.shader.version_free(resolve.shader_version);
}

void Resolve::resolve_gi(RID p_source_depth, RID p_source_normal_roughness, RID p_source_voxel_gi, RID p_dest_depth, RID p_dest_normal_roughness, RID p_dest_voxel_gi, Vector2i p_screen_size, int p_samples) {
	UniformSetCacheRD *uniform_set_cache = UniformSetCacheRD::get_singleton();
	ERR_FAIL_NULL(uniform_set_cache);
	MaterialStorage *material_storage = MaterialStorage::get_singleton();
	ERR_FAIL_NULL(material_storage);

	ResolvePushConstant push_constant;
	push_constant.screen_size[0] = p_screen_size.x;
	push_constant.screen_size[1] = p_screen_size.y;
	push_constant.samples = p_samples;

	// setup our uniforms
	RID default_sampler = material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED);

	RD::Uniform u_source_depth(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 0, Vector<RID>({ default_sampler, p_source_depth }));
	RD::Uniform u_source_normal_roughness(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 1, Vector<RID>({ default_sampler, p_source_normal_roughness }));
	RD::Uniform u_dest_depth(RD::UNIFORM_TYPE_IMAGE, 0, Vector<RID>({ p_dest_depth }));
	RD::Uniform u_dest_normal_roughness(RD::UNIFORM_TYPE_IMAGE, 1, Vector<RID>({ p_dest_normal_roughness }));

	ResolveMode mode = p_source_voxel_gi.is_valid() ? RESOLVE_MODE_GI_VOXEL_GI : RESOLVE_MODE_GI;
	RID shader = resolve.shader.version_get_shader(resolve.shader_version, mode);
	ERR_FAIL_COND(shader.is_null());

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, resolve.pipelines[mode]);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set_cache->get_cache(shader, 0, u_source_depth, u_source_normal_roughness), 0);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set_cache->get_cache(shader, 1, u_dest_depth, u_dest_normal_roughness), 1);
	if (p_source_voxel_gi.is_valid()) {
		RD::Uniform u_source_voxel_gi(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 0, Vector<RID>({ default_sampler, p_source_voxel_gi }));
		RD::Uniform u_dest_voxel_gi(RD::UNIFORM_TYPE_IMAGE, 0, p_dest_voxel_gi);

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set_cache->get_cache(shader, 2, u_source_voxel_gi), 2);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set_cache->get_cache(shader, 3, u_dest_voxel_gi), 3);
	}

	RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(ResolvePushConstant));

	RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_screen_size.x, p_screen_size.y, 1);

	RD::get_singleton()->compute_list_end();
}

void Resolve::resolve_depth(RID p_source_depth, RID p_dest_depth, Vector2i p_screen_size, int p_samples) {
	UniformSetCacheRD *uniform_set_cache = UniformSetCacheRD::get_singleton();
	ERR_FAIL_NULL(uniform_set_cache);
	MaterialStorage *material_storage = MaterialStorage::get_singleton();
	ERR_FAIL_NULL(material_storage);

	ResolvePushConstant push_constant;
	push_constant.screen_size[0] = p_screen_size.x;
	push_constant.screen_size[1] = p_screen_size.y;
	push_constant.samples = p_samples;

	// setup our uniforms
	RID default_sampler = material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED);

	RD::Uniform u_source_depth(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 0, Vector<RID>({ default_sampler, p_source_depth }));
	RD::Uniform u_dest_depth(RD::UNIFORM_TYPE_IMAGE, 0, p_dest_depth);

	ResolveMode mode = RESOLVE_MODE_DEPTH;
	RID shader = resolve.shader.version_get_shader(resolve.shader_version, mode);
	ERR_FAIL_COND(shader.is_null());

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, resolve.pipelines[mode]);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set_cache->get_cache(shader, 0, u_source_depth), 0);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set_cache->get_cache(shader, 1, u_dest_depth), 1);

	RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(ResolvePushConstant));

	RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_screen_size.x, p_screen_size.y, 1);

	RD::get_singleton()->compute_list_end();
}
