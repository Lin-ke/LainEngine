#include "renderer_gi.h"

namespace lain::RendererRD{
 GI * GI::p_singleton = nullptr;
 void GI::init()
 {
	default_voxel_gi_buffer = RD::get_singleton()->uniform_buffer_create(sizeof(VoxelGIData) * MAX_VOXEL_GI_INSTANCES);
	sdfgi_ubo = RD::get_singleton()->uniform_buffer_create(sizeof(SDFGIData));
 }
}