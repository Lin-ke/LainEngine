#include "renderer_sky.h"
#include "../storage/material_storage.h"
#include "../renderer_compositor_rd.h"
using namespace lain;
RendererRD::SkyRD::~SkyRD() {
	RendererRD::MaterialStorage *material_storage = RendererRD::MaterialStorage::get_singleton();

}

RendererRD::SkyRD::SkyRD() {
  roughness_layers = GLOBAL_GET("rendering/reflections/sky_reflections/roughness_layers");
	// sky_ggx_samples_quality = GLOBAL_GET("rendering/reflections/sky_reflections/ggx_samples");
	sky_use_cubemap_array = GLOBAL_GET("rendering/reflections/sky_reflections/texture_array_reflections");
}

float RendererRD::SkyRD::sky_get_baked_exposure(RID p_sky) const {
  Sky* sky = sky_owner.get_or_null(p_sky);
  ERR_FAIL_NULL_V(sky, 1.0);
  return sky->baked_exposure;
}

void RendererRD::SkyRD::init() {
  	RendererRD::TextureStorage *texture_storage = RendererRD::TextureStorage::get_singleton();
	RendererRD::MaterialStorage *material_storage = RendererRD::MaterialStorage::get_singleton();

	{
		// Start with the directional lights for the sky
		sky_scene_state.max_directional_lights = 4;
		uint32_t directional_light_buffer_size = sky_scene_state.max_directional_lights * sizeof(SkyDirectionalLightData);
		sky_scene_state.directional_lights = memnew_arr(SkyDirectionalLightData, sky_scene_state.max_directional_lights);
		sky_scene_state.last_frame_directional_lights = memnew_arr(SkyDirectionalLightData, sky_scene_state.max_directional_lights);
		sky_scene_state.last_frame_directional_light_count = sky_scene_state.max_directional_lights + 1;
		sky_scene_state.directional_light_buffer = RD::get_singleton()->uniform_buffer_create(directional_light_buffer_size);

		String defines = "\n#define MAX_DIRECTIONAL_LIGHT_DATA_STRUCTS " + itos(sky_scene_state.max_directional_lights) + "\n";
		defines += "\n#define SAMPLERS_BINDING_FIRST_INDEX " + itos(SAMPLERS_BINDING_FIRST_INDEX) + "\n";

		// Initialize sky
		Vector<String> sky_modes;
		sky_modes.push_back(""); // Full size
		sky_modes.push_back("\n#define USE_HALF_RES_PASS\n"); // Half Res
		sky_modes.push_back("\n#define USE_QUARTER_RES_PASS\n"); // Quarter res
		sky_modes.push_back("\n#define USE_CUBEMAP_PASS\n"); // Cubemap
		sky_modes.push_back("\n#define USE_CUBEMAP_PASS\n#define USE_HALF_RES_PASS\n"); // Half Res Cubemap
		sky_modes.push_back("\n#define USE_CUBEMAP_PASS\n#define USE_QUARTER_RES_PASS\n"); // Quarter res Cubemap

		sky_modes.push_back("\n#define USE_MULTIVIEW\n"); // Full size multiview
		sky_modes.push_back("\n#define USE_HALF_RES_PASS\n#define USE_MULTIVIEW\n"); // Half Res multiview
		sky_modes.push_back("\n#define USE_QUARTER_RES_PASS\n#define USE_MULTIVIEW\n"); // Quarter res multiview

		sky_shader.shader.initialize(sky_modes, defines);

		if (!RendererCompositorRD::get_singleton()->is_xr_enabled()) {
			sky_shader.shader.set_variant_enabled(SKY_VERSION_BACKGROUND_MULTIVIEW, false);
			sky_shader.shader.set_variant_enabled(SKY_VERSION_HALF_RES_MULTIVIEW, false);
			sky_shader.shader.set_variant_enabled(SKY_VERSION_QUARTER_RES_MULTIVIEW, false);
		}
	}
}
