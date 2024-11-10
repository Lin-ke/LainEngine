#include "renderer_compositor_rd.h"
#include "scene_shader_forward_clustered.h"
#include "storage/material_storage.h"
#include "storage/mesh_storage.h"
#include "render_forward_clustered.h"
using namespace lain;
void RendererCompositorRD::initialize() {
//   // Initialize blit
//   Vector<String> blit_modes;
//   blit_modes.push_back("\n");
//   blit_modes.push_back("\n#define USE_LAYER\n");
//   blit_modes.push_back("\n#define USE_LAYER\n#define APPLY_LENS_DISTORTION\n");
//   blit_modes.push_back("\n");

//   blit.shader.initialize(blit_modes);

//   blit.shader_version = blit.shader.version_create();

//   for (int i = 0; i < BLIT_MODE_MAX; i++) {
//     blit.pipelines[i] = RD::get_singleton()->render_pipeline_create(
//         blit.shader.version_get_shader(blit.shader_version, i), RD::get_singleton()->screen_get_framebuffer_format(DisplayServer::MAIN_WINDOW_ID), RD::INVALID_ID,
//         RD::RENDER_PRIMITIVE_TRIANGLES, RD::PipelineRasterizationState(), RD::PipelineMultisampleState(), RD::PipelineDepthStencilState(),
//         i == BLIT_MODE_NORMAL_ALPHA ? RenderingDevice::PipelineColorBlendState::create_blend() : RenderingDevice::PipelineColorBlendState::create_disabled(), 0);
//   }

//   //create index array for copy shader
//   Vector<uint8_t> pv;
//   pv.resize(6 * 2);
//   {
//     uint8_t* w = pv.ptrw();
//     uint16_t* p16 = (uint16_t*)w;
//     p16[0] = 0;
//     p16[1] = 1;
//     p16[2] = 2;
//     p16[3] = 0;
//     p16[4] = 2;
//     p16[5] = 3;
//   }
//   blit.index_buffer = RD::get_singleton()->index_buffer_create(6, RenderingDevice::INDEX_BUFFER_FORMAT_UINT16, pv);
//   blit.array = RD::get_singleton()->index_array_create(blit.index_buffer, 0, 6);

//   blit.sampler = RD::get_singleton()->sampler_create(RD::SamplerState());
}
void lain::RendererCompositorRD::begin_frame(double frame_step) {
		frame++;
	delta = frame_step;
	time += frame_step;

	double time_roll_over = GLOBAL_GET("rendering/limits/time/time_rollover_secs");
	time = Math::fmod(time, time_roll_over);

	// canvas->set_time(time);
	scene->set_time(time, frame_step);
}

void RendererCompositorRD::end_frame(bool p_swap_buffers) {
	if (p_swap_buffers) {
		RD::get_singleton()->swap_buffers();
	}
}

void RendererCompositorRD::finalize() {
	memdelete(scene);
	// memdelete(canvas);
	// memdelete(fog);
	// memdelete(particles_storage);
	memdelete(light_storage);
	memdelete(mesh_storage);
	memdelete(material_storage);
	memdelete(texture_storage);
	memdelete(utilities);

	//only need to erase these, the rest are erased by cascade
	blit.shader.version_free(blit.shader_version);
	RD::get_singleton()->free(blit.index_buffer);
	RD::get_singleton()->free(blit.sampler);
}

void lain::RendererCompositorRD::blit_render_targets_to_screen(WindowSystem::WindowID p_screen, const BlitToScreen* p_render_targets, int p_amount) {
		Error err = RD::get_singleton()->screen_prepare_for_drawing(p_screen);
	if (err != OK) {
		// Window is minimized and does not have valid swapchain, skip drawing without printing errors.
		return;
	}

	RD::DrawListID draw_list = RD::get_singleton()->draw_list_begin_for_screen(p_screen);
	ERR_FAIL_COND(draw_list == RD::INVALID_ID);
	// 绘制多个 render target
	Size2 screen_size(RD::get_singleton()->screen_get_width(p_screen), RD::get_singleton()->screen_get_height(p_screen));
	for (int i = 0; i < p_amount; i++) {
		RID rd_texture = texture_storage->render_target_get_rd_texture(p_render_targets[i].render_target);
		ERR_CONTINUE(rd_texture.is_null());

		if (!render_target_descriptors.has(rd_texture) || !RD::get_singleton()->uniform_set_is_valid(render_target_descriptors[rd_texture])) {
			Vector<RD::Uniform> uniforms;
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE; // 需要两个
			u.binding = 0;
			u.append_id(blit.sampler);
			u.append_id(rd_texture);
			uniforms.push_back(u);
			RID uniform_set = RD::get_singleton()->uniform_set_create(uniforms, blit.shader.version_get_shader(blit.shader_version, BLIT_MODE_NORMAL), 0);

			render_target_descriptors[rd_texture] = uniform_set;
		}

		BlitMode mode = p_render_targets[i].lens_distortion.apply ? BLIT_MODE_LENS : (p_render_targets[i].multi_view.use_layer ? BLIT_MODE_USE_LAYER : BLIT_MODE_NORMAL);
		RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, blit.pipelines[mode]);
		RD::get_singleton()->draw_list_bind_index_array(draw_list, blit.array);
		RD::get_singleton()->draw_list_bind_uniform_set(draw_list, render_target_descriptors[rd_texture], 0);

		blit.push_constant.src_rect[0] = p_render_targets[i].src_rect.position.x;
		blit.push_constant.src_rect[1] = p_render_targets[i].src_rect.position.y;
		blit.push_constant.src_rect[2] = p_render_targets[i].src_rect.size.width(); // z 和 w 分别是 width 和 height
		blit.push_constant.src_rect[3] = p_render_targets[i].src_rect.size.height();
		blit.push_constant.dst_rect[0] = p_render_targets[i].dst_rect.position.x / screen_size.width();
		blit.push_constant.dst_rect[1] = p_render_targets[i].dst_rect.position.y / screen_size.height();
		blit.push_constant.dst_rect[2] = p_render_targets[i].dst_rect.size.width() / screen_size.width(); // z 和 w 分别是 比例
		blit.push_constant.dst_rect[3] = p_render_targets[i].dst_rect.size.height() / screen_size.height();
		blit.push_constant.layer = p_render_targets[i].multi_view.layer;
		blit.push_constant.eye_center[0] = p_render_targets[i].lens_distortion.eye_center.x;
		blit.push_constant.eye_center[1] = p_render_targets[i].lens_distortion.eye_center.y;
		blit.push_constant.k1 = p_render_targets[i].lens_distortion.k1;
		blit.push_constant.k2 = p_render_targets[i].lens_distortion.k2;
		blit.push_constant.upscale = p_render_targets[i].lens_distortion.upscale;
		blit.push_constant.aspect_ratio = p_render_targets[i].lens_distortion.aspect_ratio;
		blit.push_constant.convert_to_srgb = texture_storage->render_target_is_using_hdr(p_render_targets[i].render_target);

		RD::get_singleton()->draw_list_set_push_constant(draw_list, &blit.push_constant, sizeof(BlitPushConstant));
		RD::get_singleton()->draw_list_draw(draw_list, true);
	}

	RD::get_singleton()->draw_list_end();
}
lain::RendererCompositorRD::RendererCompositorRD() {
  singleton = this;
  utilities = memnew(RendererRD::Utilities);
  texture_storage = memnew(RendererRD::TextureStorage);
  material_storage = memnew(RendererRD::MaterialStorage);
  mesh_storage = memnew(RendererRD::MeshStorage);
  light_storage = memnew(RendererRD::LightStorage);
  // particles_storage = memnew(RendererRD::ParticlesStorage);
  // fog = memnew(RendererRD::Fog);
  // canvas = memnew(RendererCanvasRenderRD());
  uint64_t textures_per_stage = RD::get_singleton()->limit_get(RD::LIMIT_MAX_TEXTURES_PER_SHADER_STAGE);
  if(OS::GetSingleton()->is_mobile_rdm() || textures_per_stage < 48){
      if(OS::GetSingleton()->is_forward_rdm()){
			WARN_PRINT_ONCE("Platform supports less than 48 textures per stage which is less than required by the Clustered renderer. Defaulting to Mobile renderer.");
      }

  }
  else if(OS::GetSingleton()->is_forward_rdm()){
      scene = memnew(RendererSceneRenderImplementation::RenderForwardClustered());
  } else{
    
  }

	scene->init();
}