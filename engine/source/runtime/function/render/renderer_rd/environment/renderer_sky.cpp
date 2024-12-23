#include "renderer_sky.h"
#include "../renderer_compositor_rd.h"
#include "../storage/material_storage.h"
using namespace lain;
using namespace lain::RendererRD;
#define RB_SCOPE_SKY SNAME("sky_buffers")
#define RB_HALF_TEXTURE SNAME("half_texture")
#define RB_QUARTER_TEXTURE SNAME("quarter_texture")
SkyRD::SkyRD() {
  roughness_layers = GLOBAL_GET("rendering/reflections/sky_reflections/roughness_layers");
  sky_ggx_samples_quality = GLOBAL_GET("rendering/reflections/sky_reflections/ggx_samples");
  sky_use_cubemap_array = GLOBAL_GET("rendering/reflections/sky_reflections/texture_array_reflections");
}

RendererRD::SkyRD::~SkyRD() {
  // cleanup anything created in init...
  RendererRD::MaterialStorage* material_storage = RendererRD::MaterialStorage::get_singleton();

  SkyMaterialData* md = static_cast<SkyMaterialData*>(material_storage->material_get_data(sky_shader.default_material, RendererRD::MaterialStorage::SHADER_TYPE_SKY));
  sky_shader.shader.version_free(md->shader_data->version);
  RD::get_singleton()->free(sky_scene_state.directional_light_buffer);
  RD::get_singleton()->free(sky_scene_state.uniform_buffer);
  memdelete_arr(sky_scene_state.directional_lights);
  memdelete_arr(sky_scene_state.last_frame_directional_lights);
  material_storage->shader_free(sky_shader.default_shader);
  material_storage->material_free(sky_shader.default_material);
  material_storage->shader_free(sky_scene_state.fog_shader);
  material_storage->material_free(sky_scene_state.fog_material);

  if (RD::get_singleton()->uniform_set_is_valid(sky_scene_state.uniform_set)) {
    RD::get_singleton()->free(sky_scene_state.uniform_set);
  }

  if (RD::get_singleton()->uniform_set_is_valid(sky_scene_state.default_fog_uniform_set)) {
    RD::get_singleton()->free(sky_scene_state.default_fog_uniform_set);
  }

  if (RD::get_singleton()->uniform_set_is_valid(sky_scene_state.fog_only_texture_uniform_set)) {
    RD::get_singleton()->free(sky_scene_state.fog_only_texture_uniform_set);
  }
}

float RendererRD::SkyRD::sky_get_baked_exposure(RID p_sky) const {
  Sky* sky = sky_owner.get_or_null(p_sky);
  ERR_FAIL_NULL_V(sky, 1.0);
  return sky->baked_exposure;
}

RendererRD::MaterialStorage::ShaderData* SkyRD::_create_sky_shader_func() {
  SkyShaderData* shader_data = memnew(SkyShaderData);
  return shader_data;
}

RendererRD::MaterialStorage::MaterialData* SkyRD::_create_sky_material_func(RendererRD::MaterialStorage::ShaderData* p_shader) {
  SkyMaterialData* material_data = memnew(SkyMaterialData);
  material_data->shader_data = static_cast<SkyShaderData*>(p_shader);
  //update will happen later anyway so do nothing.
  return material_data;
}

void RendererRD::SkyRD::init() {
  RendererRD::TextureStorage* texture_storage = RendererRD::TextureStorage::get_singleton();
  RendererRD::MaterialStorage* material_storage = RendererRD::MaterialStorage::get_singleton();

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
    sky_modes.push_back("");                                                            // Full size
    sky_modes.push_back("\n#define USE_HALF_RES_PASS\n");                               // Half Res
    sky_modes.push_back("\n#define USE_QUARTER_RES_PASS\n");                            // Quarter res
    sky_modes.push_back("\n#define USE_CUBEMAP_PASS\n");                                // Cubemap
    sky_modes.push_back("\n#define USE_CUBEMAP_PASS\n#define USE_HALF_RES_PASS\n");     // Half Res Cubemap
    sky_modes.push_back("\n#define USE_CUBEMAP_PASS\n#define USE_QUARTER_RES_PASS\n");  // Quarter res Cubemap

    sky_modes.push_back("\n#define USE_MULTIVIEW\n");                                // Full size multiview
    sky_modes.push_back("\n#define USE_HALF_RES_PASS\n#define USE_MULTIVIEW\n");     // Half Res multiview
    sky_modes.push_back("\n#define USE_QUARTER_RES_PASS\n#define USE_MULTIVIEW\n");  // Quarter res multiview

    sky_shader.shader.initialize(sky_modes, defines);

    if (!RendererCompositorRD::get_singleton()->is_xr_enabled()) {
      sky_shader.shader.set_variant_enabled(SKY_VERSION_BACKGROUND_MULTIVIEW, false);
      sky_shader.shader.set_variant_enabled(SKY_VERSION_HALF_RES_MULTIVIEW, false);
      sky_shader.shader.set_variant_enabled(SKY_VERSION_QUARTER_RES_MULTIVIEW, false);
    }
  }

  // register our shader funds
  material_storage->shader_set_data_request_function(RendererRD::MaterialStorage::SHADER_TYPE_SKY, _create_sky_shader_func);
  material_storage->material_set_data_request_function(RendererRD::MaterialStorage::SHADER_TYPE_SKY, _create_sky_material_func);

  {
    shader::ShaderCompiler::DefaultIdentifierActions actions;

    actions.renames["COLOR"] = "color";
    actions.renames["ALPHA"] = "alpha";
    actions.renames["EYEDIR"] = "cube_normal";
    actions.renames["POSITION"] = "params.position";
    actions.renames["SKY_COORDS"] = "panorama_coords";
    actions.renames["SCREEN_UV"] = "uv";
    actions.renames["FRAGCOORD"] = "gl_FragCoord";
    actions.renames["TIME"] = "params.time";
    actions.renames["PI"] = _MKSTR(Math_PI);
    actions.renames["TAU"] = _MKSTR(Math_TAU);
    actions.renames["E"] = _MKSTR(Math_E);
    actions.renames["HALF_RES_COLOR"] = "half_res_color";
    actions.renames["QUARTER_RES_COLOR"] = "quarter_res_color";
    actions.renames["RADIANCE"] = "radiance";
    actions.renames["FOG"] = "custom_fog";
    actions.renames["LIGHT0_ENABLED"] = "directional_lights.data[0].enabled";
    actions.renames["LIGHT0_DIRECTION"] = "directional_lights.data[0].direction_energy.xyz";
    actions.renames["LIGHT0_ENERGY"] = "directional_lights.data[0].direction_energy.w";
    actions.renames["LIGHT0_COLOR"] = "directional_lights.data[0].color_size.xyz";
    actions.renames["LIGHT0_SIZE"] = "directional_lights.data[0].color_size.w";
    actions.renames["LIGHT1_ENABLED"] = "directional_lights.data[1].enabled";
    actions.renames["LIGHT1_DIRECTION"] = "directional_lights.data[1].direction_energy.xyz";
    actions.renames["LIGHT1_ENERGY"] = "directional_lights.data[1].direction_energy.w";
    actions.renames["LIGHT1_COLOR"] = "directional_lights.data[1].color_size.xyz";
    actions.renames["LIGHT1_SIZE"] = "directional_lights.data[1].color_size.w";
    actions.renames["LIGHT2_ENABLED"] = "directional_lights.data[2].enabled";
    actions.renames["LIGHT2_DIRECTION"] = "directional_lights.data[2].direction_energy.xyz";
    actions.renames["LIGHT2_ENERGY"] = "directional_lights.data[2].direction_energy.w";
    actions.renames["LIGHT2_COLOR"] = "directional_lights.data[2].color_size.xyz";
    actions.renames["LIGHT2_SIZE"] = "directional_lights.data[2].color_size.w";
    actions.renames["LIGHT3_ENABLED"] = "directional_lights.data[3].enabled";
    actions.renames["LIGHT3_DIRECTION"] = "directional_lights.data[3].direction_energy.xyz";
    actions.renames["LIGHT3_ENERGY"] = "directional_lights.data[3].direction_energy.w";
    actions.renames["LIGHT3_COLOR"] = "directional_lights.data[3].color_size.xyz";
    actions.renames["LIGHT3_SIZE"] = "directional_lights.data[3].color_size.w";
    actions.renames["AT_CUBEMAP_PASS"] = "AT_CUBEMAP_PASS";
    actions.renames["AT_HALF_RES_PASS"] = "AT_HALF_RES_PASS";
    actions.renames["AT_QUARTER_RES_PASS"] = "AT_QUARTER_RES_PASS";
    actions.custom_samplers["RADIANCE"] = "SAMPLER_LINEAR_WITH_MIPMAPS_CLAMP";
    actions.usage_defines["HALF_RES_COLOR"] = "\n#define USES_HALF_RES_COLOR\n";
    actions.usage_defines["QUARTER_RES_COLOR"] = "\n#define USES_QUARTER_RES_COLOR\n";
    actions.render_mode_defines["disable_fog"] = "#define DISABLE_FOG\n";
    actions.render_mode_defines["use_debanding"] = "#define USE_DEBANDING\n";

    actions.base_texture_binding_index = 1;
    actions.texture_layout_set = 1;
    actions.base_uniform_string = "material.";
    actions.base_varying_index = 10;

    actions.default_filter = shader::ShaderLanguage::FILTER_LINEAR_MIPMAP;
    actions.default_repeat = shader::ShaderLanguage::REPEAT_ENABLE;
    actions.global_buffer_array_variable = "global_shader_uniforms.data";

    sky_shader.compiler.initialize(actions);
  }

  {
    // default material and shader for sky shader
    sky_shader.default_shader = material_storage->shader_allocate();
    material_storage->shader_initialize(sky_shader.default_shader);

    material_storage->shader_set_code(sky_shader.default_shader, R"(
// Default sky shader.

shader_type sky;

void sky() {
	COLOR = vec3(0.0);
}
)");

    sky_shader.default_material = material_storage->material_allocate();
    material_storage->material_initialize(sky_shader.default_material);

    material_storage->material_set_shader(sky_shader.default_material, sky_shader.default_shader);

    SkyMaterialData* md = static_cast<SkyMaterialData*>(material_storage->material_get_data(sky_shader.default_material, RendererRD::MaterialStorage::SHADER_TYPE_SKY));
    sky_shader.default_shader_rd = sky_shader.shader.version_get_shader(md->shader_data->version, SKY_VERSION_BACKGROUND);

    sky_scene_state.uniform_buffer = RD::get_singleton()->uniform_buffer_create(sizeof(SkySceneState::UBO));

    Vector<RD::Uniform> uniforms;

    {
      RD::Uniform u;
      u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
      u.binding = 1;
      u.append_id(RendererRD::MaterialStorage::get_singleton()->global_shader_uniforms_get_storage_buffer());
      uniforms.push_back(u);
    }

    {
      RD::Uniform u;
      u.binding = 2;
      u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
      u.append_id(sky_scene_state.uniform_buffer);
      uniforms.push_back(u);
    }

    {
      RD::Uniform u;
      u.binding = 3;
      u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
      u.append_id(sky_scene_state.directional_light_buffer);
      uniforms.push_back(u);
    }

    uniforms.append_array(material_storage->samplers_rd_get_default().get_uniforms(SAMPLERS_BINDING_FIRST_INDEX));

    sky_scene_state.uniform_set = RD::get_singleton()->uniform_set_create(uniforms, sky_shader.default_shader_rd, SKY_SET_UNIFORMS);
  }

  {
    Vector<RD::Uniform> uniforms;
    {
      RD::Uniform u;
      u.binding = 0;
      u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
      RID vfog = texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_3D_WHITE);
      u.append_id(vfog);
      uniforms.push_back(u);
    }

    sky_scene_state.default_fog_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, sky_shader.default_shader_rd, SKY_SET_FOG);
  }

  {
    // Need defaults for using fog with clear color
    sky_scene_state.fog_shader = material_storage->shader_allocate();
    material_storage->shader_initialize(sky_scene_state.fog_shader);

    material_storage->shader_set_code(sky_scene_state.fog_shader, R"(
// Default clear color sky shader.

shader_type sky;

uniform vec4 clear_color;

void sky() {
	COLOR = clear_color.rgb;
}
)");
    sky_scene_state.fog_material = material_storage->material_allocate();
    material_storage->material_initialize(sky_scene_state.fog_material);

    material_storage->material_set_shader(sky_scene_state.fog_material, sky_scene_state.fog_shader);

    Vector<RD::Uniform> uniforms;
    {
      RD::Uniform u;
      u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
      u.binding = 0;
      u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_CUBEMAP_BLACK));
      uniforms.push_back(u);
    }
    {
      RD::Uniform u;
      u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
      u.binding = 1;
      u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_WHITE));
      uniforms.push_back(u);
    }
    {
      RD::Uniform u;
      u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
      u.binding = 2;
      u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_WHITE));
      uniforms.push_back(u);
    }

    sky_scene_state.fog_only_texture_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, sky_shader.default_shader_rd, SKY_SET_TEXTURES);
  }
}

RID SkyRD::allocate_sky_rid() {
  return sky_owner.allocate_rid();
}

void SkyRD::initialize_sky_rid(RID p_rid) {
  sky_owner.initialize_rid(p_rid, Sky());
}

SkyRD::Sky* SkyRD::get_sky(RID p_sky) const {
  return sky_owner.get_or_null(p_sky);
}

void SkyRD::free_sky(RID p_sky) {
  Sky* sky = get_sky(p_sky);
  ERR_FAIL_NULL(sky);

  sky->free();
  sky_owner.free(p_sky);
}

void SkyRD::sky_set_radiance_size(RID p_sky, int p_radiance_size) {
  Sky* sky = get_sky(p_sky);
  ERR_FAIL_NULL(sky);

  if (sky->set_radiance_size(p_radiance_size)) {
    invalidate_sky(sky);
  }
}

void SkyRD::sky_set_mode(RID p_sky, RS::SkyMode p_mode) {
  Sky* sky = get_sky(p_sky);
  ERR_FAIL_NULL(sky);

  if (sky->set_mode(p_mode)) {
    invalidate_sky(sky);
  }
}

void SkyRD::sky_set_material(RID p_sky, RID p_material) {
  Sky* sky = get_sky(p_sky);
  ERR_FAIL_NULL(sky);

  if (sky->set_material(p_material)) {
    invalidate_sky(sky);
  }
}

Ref<Image> SkyRD::sky_bake_panorama(RID p_sky, float p_energy, bool p_bake_irradiance, const Size2i& p_size) {
  Sky* sky = get_sky(p_sky);
  ERR_FAIL_NULL_V(sky, Ref<Image>());

  update_dirty_skys();

  return sky->bake_panorama(p_energy, p_bake_irradiance ? roughness_layers : 0, p_size);
}

void SkyRD::invalidate_sky(Sky* p_sky) {
  if (!p_sky->dirty) {
    p_sky->dirty = true;
    p_sky->dirty_list = dirty_sky_list;
    dirty_sky_list = p_sky;
  }
}
void lain::RendererRD::SkyRD::update_dirty_skys() {
  Sky* sky = dirty_sky_list;

  while (sky) {
    //update sky configuration if texture is missing

    // TODO See if we can move this into `update_radiance_buffers` and remove our dirty_sky logic.
    // As this is basically a duplicate of the logic in reflection probes we could move this logic
    // into RenderSceneBuffersRD and use that from both places.
    if (sky->radiance.is_null()) {
      int mipmaps = Image::get_image_required_mipmaps(sky->radiance_size, sky->radiance_size, Image::FORMAT_RGBAH) + 1;

      uint32_t w = sky->radiance_size, h = sky->radiance_size;
      int layers = roughness_layers;
      if (sky->mode == RS::SKY_MODE_REALTIME) {
        layers = 8;
        if (roughness_layers != 8) {
          WARN_PRINT(
              "When using the Real-Time sky update mode (or Automatic with a sky shader using \"TIME\"), \"rendering/reflections/sky_reflections/roughness_layers\" should be "
              "set to 8 in the project settings for best quality reflections.");
        }
      }

      if (sky_use_cubemap_array) {
        //array (higher quality, 6 times more memory)
        RD::TextureFormat tf;
        tf.array_layers = layers * 6;
        tf.format = texture_format;
        tf.texture_type = RD::TEXTURE_TYPE_CUBE_ARRAY;
        tf.mipmaps = mipmaps;
        tf.width = w;
        tf.height = h;
        tf.usage_bits = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT;
        if (RendererSceneRenderRD::get_singleton()->_render_buffers_can_be_storage()) {
          tf.usage_bits |= RD::TEXTURE_USAGE_STORAGE_BIT;
        }

        sky->radiance = RD::get_singleton()->texture_create(tf, RD::TextureView());

        sky->reflection.update_reflection_data(sky->radiance_size, mipmaps, true, sky->radiance, 0, sky->mode == RS::SKY_MODE_REALTIME, roughness_layers, texture_format);

      } else {
        //regular cubemap, lower quality (aliasing, less memory)
        RD::TextureFormat tf;
        tf.array_layers = 6;
        tf.format = texture_format;
        tf.texture_type = RD::TEXTURE_TYPE_CUBE;
        tf.mipmaps = MIN(mipmaps, layers);
        tf.width = w;
        tf.height = h;
        tf.usage_bits = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT;
        if (RendererSceneRenderRD::get_singleton()->_render_buffers_can_be_storage()) {
          tf.usage_bits |= RD::TEXTURE_USAGE_STORAGE_BIT;
        }

        sky->radiance = RD::get_singleton()->texture_create(tf, RD::TextureView());

        sky->reflection.update_reflection_data(sky->radiance_size, MIN(mipmaps, layers), false, sky->radiance, 0, sky->mode == RS::SKY_MODE_REALTIME, roughness_layers,
                                               texture_format);
      }
    }

    sky->reflection.dirty = true;
    sky->processing_layer = 0;

    Sky* next = sky->dirty_list;
    sky->dirty_list = nullptr;
    sky->dirty = false;
    sky = next;
  }

  dirty_sky_list = nullptr;
}

RID lain::RendererRD::SkyRD::sky_get_material(RID p_sky) const {
  Sky* sky = get_sky(p_sky);
  ERR_FAIL_NULL_V(sky, RID());
  return sky->material;
}

RID SkyRD::sky_get_radiance_texture_rd(RID p_sky) const {
  Sky* sky = get_sky(p_sky);
  ERR_FAIL_NULL_V(sky, RID());

  return sky->radiance;
}

// ReflectionData
void SkyRD::ReflectionData::clear_reflection_data() {
  layers.clear();
  radiance_base_cubemap = RID();
  if (downsampled_radiance_cubemap.is_valid()) {
    RD::get_singleton()->free(downsampled_radiance_cubemap);
  }
  downsampled_radiance_cubemap = RID();
  downsampled_layer.mipmaps.clear();
  coefficient_buffer = RID();
}

void SkyRD::ReflectionData::update_reflection_data(int p_size, int p_mipmaps, bool p_use_array, RID p_base_cube, int p_base_layer, bool p_low_quality, int p_roughness_layers,
                                                   RD::DataFormat p_texture_format) {
  //recreate radiance and all data

  int mipmaps = p_mipmaps;
  uint32_t w = p_size, h = p_size;

  bool render_buffers_can_be_storage = RendererSceneRenderRD::get_singleton()->_render_buffers_can_be_storage();

  if (p_use_array) {
    int num_layers = p_low_quality ? 8 : p_roughness_layers;

    for (int i = 0; i < num_layers; i++) {
      ReflectionData::Layer layer;
      uint32_t mmw = w;
      uint32_t mmh = h;
      layer.mipmaps.resize(mipmaps);
      layer.views.resize(mipmaps);
      for (int j = 0; j < mipmaps; j++) {
        ReflectionData::Layer::Mipmap& mm = layer.mipmaps.write[j];
        mm.size.width() = mmw;
        mm.size.height() = mmh;
        for (int k = 0; k < 6; k++) {
          mm.views[k] = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), p_base_cube, p_base_layer + i * 6 + k, j);
          Vector<RID> fbtex;
          fbtex.push_back(mm.views[k]);
          mm.framebuffers[k] = RD::get_singleton()->framebuffer_create(fbtex);
        }

        layer.views.write[j] = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), p_base_cube, p_base_layer + i * 6, j, 1, RD::TEXTURE_SLICE_CUBEMAP);

        mmw = MAX(1u, mmw >> 1);
        mmh = MAX(1u, mmh >> 1);
      }

      layers.push_back(layer);
    }

  } else {
    mipmaps = p_low_quality ? 8 : mipmaps;
    //regular cubemap, lower quality (aliasing, less memory)
    ReflectionData::Layer layer;
    uint32_t mmw = w;
    uint32_t mmh = h;
    layer.mipmaps.resize(mipmaps);
    layer.views.resize(mipmaps);
    for (int j = 0; j < mipmaps; j++) {
      ReflectionData::Layer::Mipmap& mm = layer.mipmaps.write[j];
      mm.size.width() = mmw;
      mm.size.height() = mmh;
      for (int k = 0; k < 6; k++) {
        mm.views[k] = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), p_base_cube, p_base_layer + k, j);
        Vector<RID> fbtex;
        fbtex.push_back(mm.views[k]);
        mm.framebuffers[k] = RD::get_singleton()->framebuffer_create(fbtex);
      }

      layer.views.write[j] = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), p_base_cube, p_base_layer, j, 1, RD::TEXTURE_SLICE_CUBEMAP);

      mmw = MAX(1u, mmw >> 1);
      mmh = MAX(1u, mmh >> 1);
    }

    layers.push_back(layer);
  }

  radiance_base_cubemap = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), p_base_cube, p_base_layer, 0, 1, RD::TEXTURE_SLICE_CUBEMAP);
  RD::get_singleton()->set_resource_name(radiance_base_cubemap, "radiance base cubemap");

  RD::TextureFormat tf;
  tf.format = p_texture_format;
  tf.width = p_low_quality ? 64 : p_size >> 1;  // Always 64x64 when using REALTIME.
  tf.height = p_low_quality ? 64 : p_size >> 1;
  tf.texture_type = RD::TEXTURE_TYPE_CUBE;
  tf.array_layers = 6;
  tf.mipmaps = p_low_quality ? 7 : mipmaps - 1;
  tf.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;
  if (render_buffers_can_be_storage) {
    tf.usage_bits |= RD::TEXTURE_USAGE_STORAGE_BIT;
  }

  downsampled_radiance_cubemap = RD::get_singleton()->texture_create(tf, RD::TextureView());
  RD::get_singleton()->set_resource_name(downsampled_radiance_cubemap, "downsampled radiance cubemap");
  {
    uint32_t mmw = tf.width;
    uint32_t mmh = tf.height;
    downsampled_layer.mipmaps.resize(tf.mipmaps);
    for (int j = 0; j < downsampled_layer.mipmaps.size(); j++) {
      ReflectionData::DownsampleLayer::Mipmap& mm = downsampled_layer.mipmaps.write[j];
      mm.size.width() = mmw;
      mm.size.height() = mmh;
      mm.view = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), downsampled_radiance_cubemap, 0, j, 1, RD::TEXTURE_SLICE_CUBEMAP);
      RD::get_singleton()->set_resource_name(mm.view, "Downsampled Radiance Cubemap Mip " + itos(j) + " ");
      if (!render_buffers_can_be_storage) {
        // we need a framebuffer for each side of our cubemap

        for (int k = 0; k < 6; k++) {
          mm.views[k] = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), downsampled_radiance_cubemap, k, j);
          RD::get_singleton()->set_resource_name(mm.view, "Downsampled Radiance Cubemap Mip: " + itos(j) + " Face: " + itos(k) + " ");
          Vector<RID> fbtex;
          fbtex.push_back(mm.views[k]);
          mm.framebuffers[k] = RD::get_singleton()->framebuffer_create(fbtex);
        }
      }

      mmw = MAX(1u, mmw >> 1);
      mmh = MAX(1u, mmh >> 1);
    }
  }
}

void SkyRD::Sky::free() {
  if (radiance.is_valid()) {
    RD::get_singleton()->free(radiance);
    radiance = RID();
  }
  reflection.clear_reflection_data();

  if (uniform_buffer.is_valid()) {
    RD::get_singleton()->free(uniform_buffer);
    uniform_buffer = RID();
  }

  if (material.is_valid()) {
    RSG::material_storage->material_free(material);
    material = RID();
  }
}

RID SkyRD::Sky::get_textures(SkyTextureSetVersion p_version, RID p_default_shader_rd, Ref<RenderSceneBuffersRD> p_render_buffers) {
  RendererRD::TextureStorage* texture_storage = RendererRD::TextureStorage::get_singleton();

  Vector<RD::Uniform> uniforms;
  {
    RD::Uniform u;
    u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
    u.binding = 0;
    if (radiance.is_valid() && p_version <= SKY_TEXTURE_SET_QUARTER_RES) {
      u.append_id(radiance);
    } else {
      u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_CUBEMAP_BLACK));
    }
    uniforms.push_back(u);
  }
  {
    RD::Uniform u;
    u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
    u.binding = 1;  // half res
    if (p_version >= SKY_TEXTURE_SET_CUBEMAP) {
      if (reflection.layers.size() && reflection.layers[0].views.size() >= 2 && reflection.layers[0].views[1].is_valid() && p_version != SKY_TEXTURE_SET_CUBEMAP_HALF_RES) {
        u.append_id(reflection.layers[0].views[1]);
      } else {
        u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_CUBEMAP_BLACK));
      }
    } else {
      RID half_texture = p_render_buffers->has_texture(RB_SCOPE_SKY, RB_HALF_TEXTURE) ? p_render_buffers->get_texture(RB_SCOPE_SKY, RB_HALF_TEXTURE) : RID();
      if (half_texture.is_valid() && p_version != SKY_TEXTURE_SET_HALF_RES) {
        u.append_id(half_texture);
      } else {
        u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_WHITE));
      }
    }
    uniforms.push_back(u);
  }
  {
    RD::Uniform u;
    u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
    u.binding = 2;  // quarter res
    if (p_version >= SKY_TEXTURE_SET_CUBEMAP) {
      if (reflection.layers.size() && reflection.layers[0].views.size() >= 3 && reflection.layers[0].views[2].is_valid() && p_version != SKY_TEXTURE_SET_CUBEMAP_QUARTER_RES) {
        u.append_id(reflection.layers[0].views[2]);
      } else {
        u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_CUBEMAP_BLACK));
      }
    } else {
      RID quarter_texture = p_render_buffers->has_texture(RB_SCOPE_SKY, RB_QUARTER_TEXTURE) ? p_render_buffers->get_texture(RB_SCOPE_SKY, RB_QUARTER_TEXTURE) : RID();
      if (quarter_texture.is_valid() && p_version != SKY_TEXTURE_SET_QUARTER_RES) {
        u.append_id(quarter_texture);
      } else {
        u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_WHITE));
      }
    }
    uniforms.push_back(u);
  }

  return UniformSetCacheRD::get_singleton()->get_cache_vec(p_default_shader_rd, SKY_SET_TEXTURES, uniforms);
}

// 这边代码和material 那边是一样的

void SkyRD::SkyShaderData::set_code(const String& p_code) {
  //compile

  code = p_code;
  valid = false;
  ubo_size = 0;
  uniforms.clear();

  if (code.is_empty()) {
    return;  //just invalid, but no error
  }

  shader::ShaderCompiler::GeneratedCode gen_code;
  shader::ShaderCompiler::IdentifierActions actions;
  actions.entry_point_stages["sky"] = shader::ShaderCompiler::STAGE_FRAGMENT;

  uses_time = false;
  uses_half_res = false;
  uses_quarter_res = false;
  uses_position = false;
  uses_light = false;

  actions.render_mode_flags["use_half_res_pass"] = &uses_half_res;
  actions.render_mode_flags["use_quarter_res_pass"] = &uses_quarter_res;

  actions.usage_flag_pointers["TIME"] = &uses_time;
  actions.usage_flag_pointers["POSITION"] = &uses_position;
  actions.usage_flag_pointers["LIGHT0_ENABLED"] = &uses_light;
  actions.usage_flag_pointers["LIGHT0_ENERGY"] = &uses_light;
  actions.usage_flag_pointers["LIGHT0_DIRECTION"] = &uses_light;
  actions.usage_flag_pointers["LIGHT0_COLOR"] = &uses_light;
  actions.usage_flag_pointers["LIGHT0_SIZE"] = &uses_light;
  actions.usage_flag_pointers["LIGHT1_ENABLED"] = &uses_light;
  actions.usage_flag_pointers["LIGHT1_ENERGY"] = &uses_light;
  actions.usage_flag_pointers["LIGHT1_DIRECTION"] = &uses_light;
  actions.usage_flag_pointers["LIGHT1_COLOR"] = &uses_light;
  actions.usage_flag_pointers["LIGHT1_SIZE"] = &uses_light;
  actions.usage_flag_pointers["LIGHT2_ENABLED"] = &uses_light;
  actions.usage_flag_pointers["LIGHT2_ENERGY"] = &uses_light;
  actions.usage_flag_pointers["LIGHT2_DIRECTION"] = &uses_light;
  actions.usage_flag_pointers["LIGHT2_COLOR"] = &uses_light;
  actions.usage_flag_pointers["LIGHT2_SIZE"] = &uses_light;
  actions.usage_flag_pointers["LIGHT3_ENABLED"] = &uses_light;
  actions.usage_flag_pointers["LIGHT3_ENERGY"] = &uses_light;
  actions.usage_flag_pointers["LIGHT3_DIRECTION"] = &uses_light;
  actions.usage_flag_pointers["LIGHT3_COLOR"] = &uses_light;
  actions.usage_flag_pointers["LIGHT3_SIZE"] = &uses_light;

  actions.uniforms = &uniforms;

  // !BAS! Contemplate making `SkyShader sky` accessible from this struct or even part of this struct.
  RendererSceneRenderRD* scene_singleton = static_cast<RendererSceneRenderRD*>(RendererSceneRenderRD::singleton);

  Error err = scene_singleton->sky.sky_shader.compiler.compile(RS::SHADER_SKY, code, &actions, path, gen_code);
  ERR_FAIL_COND_MSG(err != OK, "Shader compilation failed.");

  if (version.is_null()) {
    version = scene_singleton->sky.sky_shader.shader.version_create();
  }

#if 1
  print_line("**compiling shader:");
  print_line("**defines:\n");
  for (int i = 0; i < gen_code.defines.size(); i++) {
    print_line(gen_code.defines[i]);
  }

  HashMap<String, String>::Iterator el = gen_code.code.begin();
  while (el) {
    print_line("\n**code " + el->key + ":\n" + el->value);
    ++el;
  }

  print_line("\n**uniforms:\n" + gen_code.uniforms);
  print_line("\n**vertex_globals:\n" + gen_code.stage_globals[shader::ShaderCompiler::STAGE_VERTEX]);
  print_line("\n**fragment_globals:\n" + gen_code.stage_globals[shader::ShaderCompiler::STAGE_FRAGMENT]);
#endif

  scene_singleton->sky.sky_shader.shader.version_set_code(version, gen_code.code, gen_code.uniforms, gen_code.stage_globals[shader::ShaderCompiler::STAGE_VERTEX],
                                                          gen_code.stage_globals[shader::ShaderCompiler::STAGE_FRAGMENT], gen_code.defines);
  ERR_FAIL_COND(!scene_singleton->sky.sky_shader.shader.version_is_valid(version));

  ubo_size = gen_code.uniform_total_size;
  ubo_offsets = gen_code.uniform_offsets;
  texture_uniforms = gen_code.texture_uniforms;

  //update pipelines

  for (int i = 0; i < SKY_VERSION_MAX; i++) {
    RD::PipelineDepthStencilState depth_stencil_state;
    depth_stencil_state.enable_depth_test = true;
    depth_stencil_state.depth_compare_operator = RD::COMPARE_OP_GREATER_OR_EQUAL;

    if (scene_singleton->sky.sky_shader.shader.is_variant_enabled(i)) {
      RID shader_variant = scene_singleton->sky.sky_shader.shader.version_get_shader(version, i);
      pipelines[i].setup(shader_variant, RD::RENDER_PRIMITIVE_TRIANGLES, RD::PipelineRasterizationState(), RD::PipelineMultisampleState(), depth_stencil_state,
                         RD::PipelineColorBlendState::create_disabled(), 0);
    } else {
      pipelines[i].clear();
    }
  }

  valid = true;
}

bool SkyRD::SkyShaderData::is_animated() const {
  return false;
}

bool SkyRD::SkyShaderData::casts_shadows() const {
  return false;
}

RS::ShaderNativeSourceCode SkyRD::SkyShaderData::get_native_source_code() const {
  RendererSceneRenderRD* scene_singleton = static_cast<RendererSceneRenderRD*>(RendererSceneRenderRD::singleton);

  return scene_singleton->sky.sky_shader.shader.version_get_native_source_code(version);
}

lain::RendererRD::SkyRD::SkyShaderData::~SkyShaderData() {
  RendererSceneRenderRD* scene_singleton = static_cast<RendererSceneRenderRD*>(RendererSceneRenderRD::singleton);
  ERR_FAIL_NULL(scene_singleton);
  //pipeline variants will clear themselves if shader is gone
  if (version.is_valid()) {
    scene_singleton->sky.sky_shader.shader.version_free(version);
  }
}

Ref<Image> SkyRD::Sky::bake_panorama(float p_energy, int p_roughness_layers, const Size2i& p_size) {
  if (radiance.is_valid()) {
    RendererRD::CopyEffects* copy_effects = RendererRD::CopyEffects::get_singleton();

    RD::TextureFormat tf;
    tf.format = RD::DATA_FORMAT_R32G32B32A32_SFLOAT;  // Could be RGBA16
    tf.width = p_size.width();
    tf.height = p_size.height();
    tf.usage_bits = RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;

    RID rad_tex = RD::get_singleton()->texture_create(tf, RD::TextureView());
    copy_effects->copy_cubemap_to_panorama(radiance, rad_tex, p_size, p_roughness_layers, reflection.layers.size() > 1);
    Vector<uint8_t> data = RD::get_singleton()->texture_get_data(rad_tex, 0);
    RD::get_singleton()->free(rad_tex);

    Ref<Image> img = Image::create_from_data(p_size.width(), p_size.height(), false, Image::FORMAT_RGBAF, data);
    for (int i = 0; i < p_size.width(); i++) {
      for (int j = 0; j < p_size.height(); j++) {
        Color c = img->get_pixel(i, j);
        c.r *= p_energy;
        c.g *= p_energy;
        c.b *= p_energy;
        img->set_pixel(i, j, c);
      }
    }
    return img;
  }

  return Ref<Image>();
}

bool lain::RendererRD::SkyRD::SkyMaterialData::update_parameters(const HashMap<StringName, Variant>& p_parameters, bool p_uniform_dirty, bool p_textures_dirty) {
  RendererSceneRenderRD* scene_singleton = static_cast<RendererSceneRenderRD*>(RendererSceneRenderRD::singleton);

  uniform_set_updated = true;

  return update_parameters_uniform_set(p_parameters, p_uniform_dirty, p_textures_dirty, shader_data->uniforms, shader_data->ubo_offsets.ptr(), shader_data->texture_uniforms,
                                       shader_data->default_texture_params, shader_data->ubo_size, uniform_set,
                                       scene_singleton->sky.sky_shader.shader.version_get_shader(shader_data->version, 0), SKY_SET_MATERIAL, true, true);
}

bool SkyRD::Sky::set_radiance_size(int p_radiance_size) {
	ERR_FAIL_COND_V(p_radiance_size < 32 || p_radiance_size > 2048, false);
	if (radiance_size == p_radiance_size) {
		return false;
	}
	radiance_size = p_radiance_size;

	if (mode == RS::SKY_MODE_REALTIME && radiance_size != 256) {
		WARN_PRINT("Realtime Skies can only use a radiance size of 256. Radiance size will be set to 256 internally.");
		radiance_size = 256;
	}

	if (radiance.is_valid()) {
		RD::get_singleton()->free(radiance);
		radiance = RID();
	}
	reflection.clear_reflection_data();

	return true;
}


bool SkyRD::Sky::set_mode(RS::SkyMode p_mode) {
	if (mode == p_mode) {
		return false;
	}

	mode = p_mode;

	if (mode == RS::SKY_MODE_REALTIME && radiance_size != 256) {
		WARN_PRINT("Realtime Skies can only use a radiance size of 256. Radiance size will be set to 256 internally.");
		set_radiance_size(256);
	}

	if (radiance.is_valid()) {
		RD::get_singleton()->free(radiance);
		radiance = RID();
	}
	reflection.clear_reflection_data();

	return true;
}

bool SkyRD::Sky::set_material(RID p_material) {
	if (material == p_material) {
		return false;
	}

	material = p_material;
	return true;
}

SkyRD::SkyMaterialData::~SkyMaterialData() {
	free_parameters_uniform_set(uniform_set);
}