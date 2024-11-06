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