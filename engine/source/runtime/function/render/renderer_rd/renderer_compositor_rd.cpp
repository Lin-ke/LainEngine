#include "renderer_compositor_rd.h"
#include "storage/material_storage.h"
#include "storage/mesh_storage.h"
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
  material_storage = new (RendererRD::MaterialStorage);
  texture_storage = new (RendererRD::TextureStorage);
  mesh_storage = new (RendererRD::MeshStorage);
  light_storage = new (RendererRD::LightStorage);

}