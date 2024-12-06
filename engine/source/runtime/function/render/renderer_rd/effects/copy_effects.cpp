#include "copy_effects.h"
#include "function/render/renderer_rd/storage/material_storage.h"
#include "function/render/renderer_rd/uniform_set_cache_rd.h"
#include "../storage/framebuffer_cache_rd.h"
using namespace lain;
using namespace lain::RendererRD;
CopyEffects* CopyEffects::singleton = nullptr;

CopyEffects* CopyEffects::get_singleton() {
  return singleton;
}

lain::RendererRD::CopyEffects::CopyEffects(bool p_prefer_raster_effects) {
  singleton = this;
  prefer_raster_effects = p_prefer_raster_effects;
  // if (prefer_raster_effects) {
  //   // init blur shader (on compute use copy shader)

  //   Vector<String> blur_modes;
  //   blur_modes.push_back("\n#define MODE_MIPMAP\n");                                         // BLUR_MIPMAP
  //   blur_modes.push_back("\n#define MODE_GAUSSIAN_BLUR\n");                                  // BLUR_MODE_GAUSSIAN_BLUR
  //   blur_modes.push_back("\n#define MODE_GAUSSIAN_GLOW\n");                                  // BLUR_MODE_GAUSSIAN_GLOW
  //   blur_modes.push_back("\n#define MODE_GAUSSIAN_GLOW\n#define GLOW_USE_AUTO_EXPOSURE\n");  // BLUR_MODE_GAUSSIAN_GLOW_AUTO_EXPOSURE
  //   blur_modes.push_back("\n#define MODE_COPY\n");                                           // BLUR_MODE_COPY
  //   blur_modes.push_back("\n#define MODE_SET_COLOR\n");                                      // BLUR_MODE_SET_COLOR

  //   blur_raster.shader.initialize(blur_modes);
  //   memset(&blur_raster.push_constant, 0, sizeof(BlurRasterPushConstant));
  //   blur_raster.shader_version = blur_raster.shader.version_create();

  //   for (int i = 0; i < BLUR_MODE_MAX; i++) {
  //     blur_raster.pipelines[i].setup(blur_raster.shader.version_get_shader(blur_raster.shader_version, i), RD::RENDER_PRIMITIVE_TRIANGLES, RD::PipelineRasterizationState(),
  //                                    RD::PipelineMultisampleState(), RD::PipelineDepthStencilState(), RD::PipelineColorBlendState::create_disabled(), 0);
  //   }

  // } else {
  //   // not used in clustered
  //   for (int i = 0; i < BLUR_MODE_MAX; i++) {
  //     blur_raster.pipelines[i].clear();
  //   }
  // }

  {
    Vector<String> copy_modes;
    copy_modes.push_back("\n#define MODE_GAUSSIAN_BLUR\n");
    copy_modes.push_back("\n#define MODE_GAUSSIAN_BLUR\n#define DST_IMAGE_8BIT\n");
    copy_modes.push_back("\n#define MODE_GAUSSIAN_BLUR\n#define MODE_GLOW\n");
    copy_modes.push_back("\n#define MODE_GAUSSIAN_BLUR\n#define MODE_GLOW\n#define GLOW_USE_AUTO_EXPOSURE\n");
    copy_modes.push_back("\n#define MODE_SIMPLE_COPY\n");
    copy_modes.push_back("\n#define MODE_SIMPLE_COPY\n#define DST_IMAGE_8BIT\n");
    copy_modes.push_back("\n#define MODE_SIMPLE_COPY_DEPTH\n");
    copy_modes.push_back("\n#define MODE_SET_COLOR\n");
    copy_modes.push_back("\n#define MODE_SET_COLOR\n#define DST_IMAGE_8BIT\n");
    copy_modes.push_back("\n#define MODE_MIPMAP\n");
    copy_modes.push_back("\n#define MODE_LINEARIZE_DEPTH_COPY\n");
    copy_modes.push_back("\n#define MODE_CUBEMAP_TO_PANORAMA\n");
    copy_modes.push_back("\n#define MODE_CUBEMAP_ARRAY_TO_PANORAMA\n");

    copy.shader.initialize(copy_modes);
    memset(&copy.push_constant, 0, sizeof(CopyPushConstant));

    copy.shader_version = copy.shader.version_create();

    for (int i = 0; i < COPY_MODE_MAX; i++) {
      if (copy.shader.is_variant_enabled(i)) {
        copy.pipelines[i] = RD::get_singleton()->compute_pipeline_create(copy.shader.version_get_shader(copy.shader_version, i));
      }
    }
  }

  // {
  // 	Vector<String> copy_modes;
  // 	copy_modes.push_back("\n"); // COPY_TO_FB_COPY
  // 	copy_modes.push_back("\n#define MODE_PANORAMA_TO_DP\n"); // COPY_TO_FB_COPY_PANORAMA_TO_DP
  // 	copy_modes.push_back("\n#define MODE_TWO_SOURCES\n"); // COPY_TO_FB_COPY2
  // 	copy_modes.push_back("\n#define MODE_SET_COLOR\n"); // COPY_TO_FB_SET_COLOR
  // 	copy_modes.push_back("\n#define USE_MULTIVIEW\n"); // COPY_TO_FB_MULTIVIEW
  // 	copy_modes.push_back("\n#define USE_MULTIVIEW\n#define MODE_TWO_SOURCES\n"); // COPY_TO_FB_MULTIVIEW_WITH_DEPTH

  // 	copy_to_fb.shader.initialize(copy_modes);

  // 	if (!RendererCompositorRD::get_singleton()->is_xr_enabled()) {
  // 		copy_to_fb.shader.set_variant_enabled(COPY_TO_FB_MULTIVIEW, false);
  // 		copy_to_fb.shader.set_variant_enabled(COPY_TO_FB_MULTIVIEW_WITH_DEPTH, false);
  // 	}

  // 	copy_to_fb.shader_version = copy_to_fb.shader.version_create();

  // 	//use additive

  // 	for (int i = 0; i < COPY_TO_FB_MAX; i++) {
  // 		if (copy_to_fb.shader.is_variant_enabled(i)) {
  // 			copy_to_fb.pipelines[i].setup(copy_to_fb.shader.version_get_shader(copy_to_fb.shader_version, i), RD::RENDER_PRIMITIVE_TRIANGLES, RD::PipelineRasterizationState(), RD::PipelineMultisampleState(), RD::PipelineDepthStencilState(), RD::PipelineColorBlendState::create_disabled(), 0);
  // 		} else {
  // 			copy_to_fb.pipelines[i].clear();
  // 		}
  // 	}
  // }
}
lain::RendererRD::CopyEffects::~CopyEffects() {}
void CopyEffects::copy_to_rect(RID p_source_rd_texture, RID p_dest_texture, const Rect2i& p_rect, bool p_flip_y, bool p_force_luminance, bool p_all_source, bool p_8_bit_dst,
                               bool p_alpha_to_one) {
  UniformSetCacheRD* uniform_set_cache = UniformSetCacheRD::get_singleton();
  ERR_FAIL_NULL(uniform_set_cache);
  MaterialStorage* material_storage = MaterialStorage::get_singleton();
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

  RD::Uniform u_source_rd_texture(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 0, Vector<RID>({default_sampler, p_source_rd_texture}));
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

void CopyEffects::copy_to_fb_rect(RID p_source_rd_texture, RID p_dest_framebuffer, const Rect2i& p_rect, bool p_flip_y, bool p_force_luminance, bool p_alpha_to_zero,
                                  bool p_srgb, RID p_secondary, bool p_multiview, bool p_alpha_to_one, bool p_linear, bool p_normal, const Rect2& p_src_rect) {
  UniformSetCacheRD* uniform_set_cache = UniformSetCacheRD::get_singleton();
  ERR_FAIL_NULL(uniform_set_cache);
  MaterialStorage* material_storage = MaterialStorage::get_singleton();
  ERR_FAIL_NULL(material_storage);

  memset(&copy_to_fb.push_constant, 0, sizeof(CopyToFbPushConstant));
  copy_to_fb.push_constant.luminance_multiplier = 1.0;

  if (p_flip_y) {
    copy_to_fb.push_constant.flags |= COPY_TO_FB_FLAG_FLIP_Y;
  }
  if (p_force_luminance) {
    copy_to_fb.push_constant.flags |= COPY_TO_FB_FLAG_FORCE_LUMINANCE;
  }
  if (p_alpha_to_zero) {
    copy_to_fb.push_constant.flags |= COPY_TO_FB_FLAG_ALPHA_TO_ZERO;
  }
  if (p_srgb) {
    copy_to_fb.push_constant.flags |= COPY_TO_FB_FLAG_SRGB;
  }
  if (p_alpha_to_one) {
    copy_to_fb.push_constant.flags |= COPY_TO_FB_FLAG_ALPHA_TO_ONE;
  }
  if (p_linear) {
    // Used for copying to a linear buffer. In the mobile renderer we divide the contents of the linear buffer
    // to allow for a wider effective range.
    copy_to_fb.push_constant.flags |= COPY_TO_FB_FLAG_LINEAR;
    copy_to_fb.push_constant.luminance_multiplier = prefer_raster_effects ? 2.0 : 1.0;
  }

  if (p_normal) {
    copy_to_fb.push_constant.flags |= COPY_TO_FB_FLAG_NORMAL;
  }

  if (p_src_rect != Rect2()) {
    copy_to_fb.push_constant.section[0] = p_src_rect.position.x;
    copy_to_fb.push_constant.section[1] = p_src_rect.position.y;
    copy_to_fb.push_constant.section[2] = p_src_rect.size.x;
    copy_to_fb.push_constant.section[3] = p_src_rect.size.y;
    copy_to_fb.push_constant.flags |= COPY_TO_FB_FLAG_USE_SRC_SECTION;
  }

  // setup our uniforms
  RID default_sampler = material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED);

  RD::Uniform u_source_rd_texture(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 0, Vector<RID>({default_sampler, p_source_rd_texture}));

  CopyToFBMode mode;
  if (p_multiview) {
    mode = p_secondary.is_valid() ? COPY_TO_FB_MULTIVIEW_WITH_DEPTH : COPY_TO_FB_MULTIVIEW;
  } else {
    mode = p_secondary.is_valid() ? COPY_TO_FB_COPY2 : COPY_TO_FB_COPY;
  }

  RID shader = copy_to_fb.shader.version_get_shader(copy_to_fb.shader_version, mode);
  ERR_FAIL_COND(shader.is_null());

  RD::DrawListID draw_list = RD::get_singleton()->draw_list_begin(p_dest_framebuffer, RD::ColorInitialAction::create_load(), RD::ColorFinalAction(), RD::INITIAL_ACTION_LOAD,
                                                                  RD::FINAL_ACTION_DISCARD, Vector<Color>(), 0.0, 0, p_rect);
  RD::get_singleton()->draw_list_bind_render_pipeline(
      draw_list, copy_to_fb.pipelines[mode].get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(p_dest_framebuffer)));
  RD::get_singleton()->draw_list_bind_uniform_set(draw_list, uniform_set_cache->get_cache(shader, 0, u_source_rd_texture), 0);
  if (p_secondary.is_valid()) {
    // TODO may need to do this differently when reading from depth buffer for multiview
    RD::Uniform u_secondary(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 0, Vector<RID>({default_sampler, p_secondary}));
    RD::get_singleton()->draw_list_bind_uniform_set(draw_list, uniform_set_cache->get_cache(shader, 1, u_secondary), 1);
  }
  RD::get_singleton()->draw_list_bind_index_array(draw_list, material_storage->get_quad_index_array());
  RD::get_singleton()->draw_list_set_push_constant(draw_list, &copy_to_fb.push_constant, sizeof(CopyToFbPushConstant));
  RD::get_singleton()->draw_list_draw(draw_list, true);
  RD::get_singleton()->draw_list_end();
}

void CopyEffects::copy_cubemap_to_dp(RID p_source_rd_texture, RID p_dst_framebuffer, const Rect2 &p_rect, const Vector2 &p_dst_size, float p_z_near, float p_z_far, bool p_dp_flip) {
	UniformSetCacheRD *uniform_set_cache = UniformSetCacheRD::get_singleton();
	ERR_FAIL_NULL(uniform_set_cache);
	MaterialStorage *material_storage = MaterialStorage::get_singleton();
	ERR_FAIL_NULL(material_storage);

	CopyToDPPushConstant push_constant;
	push_constant.screen_rect[0] = p_rect.position.x;
	push_constant.screen_rect[1] = p_rect.position.y;
	push_constant.screen_rect[2] = p_rect.size.width();
	push_constant.screen_rect[3] = p_rect.size.height();
	push_constant.z_far = p_z_far;
	push_constant.z_near = p_z_near;
	push_constant.texel_size[0] = 1.0f / p_dst_size.x;
	push_constant.texel_size[1] = 1.0f / p_dst_size.y;
	push_constant.texel_size[0] *= p_dp_flip ? -1.0f : 1.0f; // Encode dp flip as x size sign

	// setup our uniforms
	RID default_sampler = material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED);

	RD::Uniform u_source_rd_texture(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 0, Vector<RID>({ default_sampler, p_source_rd_texture }));

	RID shader = cube_to_dp.shader.version_get_shader(cube_to_dp.shader_version, 0);
	ERR_FAIL_COND(shader.is_null());

	RD::DrawListID draw_list = RD::get_singleton()->draw_list_begin(p_dst_framebuffer, RD::INITIAL_ACTION_DISCARD, RD::FINAL_ACTION_DISCARD, RD::INITIAL_ACTION_LOAD, RD::FINAL_ACTION_STORE);
	RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, cube_to_dp.pipeline.get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(p_dst_framebuffer)));
	RD::get_singleton()->draw_list_bind_uniform_set(draw_list, uniform_set_cache->get_cache(shader, 0, u_source_rd_texture), 0);
	RD::get_singleton()->draw_list_bind_index_array(draw_list, material_storage->get_quad_index_array());

	RD::get_singleton()->draw_list_set_push_constant(draw_list, &push_constant, sizeof(CopyToDPPushConstant));
	RD::get_singleton()->draw_list_draw(draw_list, true);
	RD::get_singleton()->draw_list_end();
}

void CopyEffects::make_mipmap(RID p_source_rd_texture, RID p_dest_texture, const Size2i &p_size) {
	ERR_FAIL_COND_MSG(prefer_raster_effects, "Can't use the compute version of the make_mipmap shader with the mobile renderer.");

	UniformSetCacheRD *uniform_set_cache = UniformSetCacheRD::get_singleton();
	ERR_FAIL_NULL(uniform_set_cache);
	MaterialStorage *material_storage = MaterialStorage::get_singleton();
	ERR_FAIL_NULL(material_storage);

	memset(&copy.push_constant, 0, sizeof(CopyPushConstant));

	copy.push_constant.section[0] = 0;
	copy.push_constant.section[1] = 0;
	copy.push_constant.section[2] = p_size.width();
	copy.push_constant.section[3] = p_size.height();

	// setup our uniforms
	RID default_sampler = material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED);

	RD::Uniform u_source_rd_texture(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 0, Vector<RID>({ default_sampler, p_source_rd_texture }));
	RD::Uniform u_dest_texture(RD::UNIFORM_TYPE_IMAGE, 0, p_dest_texture);

	CopyMode mode = COPY_MODE_MIPMAP;
	RID shader = copy.shader.version_get_shader(copy.shader_version, mode);
	ERR_FAIL_COND(shader.is_null());

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, copy.pipelines[mode]);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set_cache->get_cache(shader, 0, u_source_rd_texture), 0);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set_cache->get_cache(shader, 3, u_dest_texture), 3);
	RD::get_singleton()->compute_list_set_push_constant(compute_list, &copy.push_constant, sizeof(CopyPushConstant));
	RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_size.width(), p_size.height(), 1);
	RD::get_singleton()->compute_list_end();
}

// void CopyEffects::make_mipmap_raster(RID p_source_rd_texture, RID p_dest_texture, const Size2i &p_size) {
// 	ERR_FAIL_COND_MSG(!prefer_raster_effects, "Can't use the raster version of mipmap with the clustered renderer.");

// 	RID dest_framebuffer = FramebufferCacheRD::get_singleton()->get_cache(p_dest_texture);

// 	UniformSetCacheRD *uniform_set_cache = UniformSetCacheRD::get_singleton();
// 	ERR_FAIL_NULL(uniform_set_cache);
// 	MaterialStorage *material_storage = MaterialStorage::get_singleton();
// 	ERR_FAIL_NULL(material_storage);

// 	memset(&blur_raster.push_constant, 0, sizeof(BlurRasterPushConstant));

// 	BlurRasterMode mode = BLUR_MIPMAP;

// 	blur_raster.push_constant.pixel_size[0] = 1.0 / float(p_size.x);
// 	blur_raster.push_constant.pixel_size[1] = 1.0 / float(p_size.y);

// 	// setup our uniforms
// 	RID default_sampler = material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED);

// 	RD::Uniform u_source_rd_texture(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 0, Vector<RID>({ default_sampler, p_source_rd_texture }));

// 	RID shader = blur_raster.shader.version_get_shader(blur_raster.shader_version, mode);
// 	ERR_FAIL_COND(shader.is_null());

// 	RD::DrawListID draw_list = RD::get_singleton()->draw_list_begin(dest_framebuffer, RD::INITIAL_ACTION_LOAD, RD::FINAL_ACTION_STORE, RD::INITIAL_ACTION_LOAD, RD::FINAL_ACTION_DISCARD);
// 	RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, blur_raster.pipelines[mode].get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(dest_framebuffer)));
// 	RD::get_singleton()->draw_list_bind_uniform_set(draw_list, uniform_set_cache->get_cache(shader, 0, u_source_rd_texture), 0);
// 	RD::get_singleton()->draw_list_set_push_constant(draw_list, &blur_raster.push_constant, sizeof(BlurRasterPushConstant));

// 	RD::get_singleton()->draw_list_draw(draw_list, false, 1u, 3u);
// 	RD::get_singleton()->draw_list_end();
// }