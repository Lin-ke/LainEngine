#include "rendering_system.h"
using namespace lain;
RenderingSystem* RenderingSystem::p_singleton = nullptr;

//
void lain::RenderingSystem::init() {
  // These are overrides, even if they are false Godot will still
  // import the texture formats that the host platform needs.
  // See `const bool can_s3tc_bptc` in the resource importer.
  GLOBAL_DEF_RST("rendering/textures/vram_compression/import_s3tc_bptc", false);
  GLOBAL_DEF_RST("rendering/textures/vram_compression/import_etc2_astc", false);
  GLOBAL_DEF("rendering/textures/vram_compression/compress_with_gpu", true);

  GLOBAL_DEF("rendering/textures/lossless_compression/force_png", false);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/textures/webp_compression/compression_method", PROPERTY_HINT_RANGE, "0,6,1"), 2);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/textures/webp_compression/lossless_compression_factor", PROPERTY_HINT_RANGE, "0,100,1"), 25);

  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/limits/time/time_rollover_secs", PROPERTY_HINT_RANGE, "0,10000,1,or_greater"), 3600);

  GLOBAL_DEF_RST("rendering/lights_and_shadows/use_physical_light_units", false);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/lights_and_shadows/directional_shadow/size", PROPERTY_HINT_RANGE, "256,16384"), 4096);
  GLOBAL_DEF("rendering/lights_and_shadows/directional_shadow/size.mobile", 2048);
  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/lights_and_shadows/directional_shadow/soft_shadow_filter_quality", PROPERTY_HINT_ENUM,
                          "Hard (Fastest),Soft Very Low (Faster),Soft Low (Fast),Soft Medium (Average),Soft High (Slow),Soft Ultra (Slowest)"),
             2);
  GLOBAL_DEF("rendering/lights_and_shadows/directional_shadow/soft_shadow_filter_quality.mobile", 0);
  GLOBAL_DEF("rendering/lights_and_shadows/directional_shadow/16_bits", true);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/lights_and_shadows/positional_shadow/soft_shadow_filter_quality", PROPERTY_HINT_ENUM,
                          "Hard (Fastest),Soft Very Low (Faster),Soft Low (Fast),Soft Medium (Average),Soft High (Slow),Soft Ultra (Slowest)"),
             2);
  GLOBAL_DEF("rendering/lights_and_shadows/positional_shadow/soft_shadow_filter_quality.mobile", 0);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/2d/shadow_atlas/size", PROPERTY_HINT_RANGE, "128,16384"), 2048);

  // Number of commands that can be drawn per frame.
  GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/gl_compatibility/item_buffer_size", PROPERTY_HINT_RANGE, "128,1048576,1"), 16384);

  GLOBAL_DEF("rendering/shader_compiler/shader_cache/enabled", true);
  GLOBAL_DEF("rendering/shader_compiler/shader_cache/compress", true);
  GLOBAL_DEF("rendering/shader_compiler/shader_cache/use_zstd_compression", true);
  GLOBAL_DEF("rendering/shader_compiler/shader_cache/strip_debug", false);
  GLOBAL_DEF("rendering/shader_compiler/shader_cache/strip_debug.release", true);

  GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/reflections/sky_reflections/roughness_layers", PROPERTY_HINT_RANGE, "1,32,1"), 8);  // Assumes a 256x256 cubemap
  GLOBAL_DEF_RST("rendering/reflections/sky_reflections/texture_array_reflections", true);
  GLOBAL_DEF("rendering/reflections/sky_reflections/texture_array_reflections.mobile", false);
  GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/reflections/sky_reflections/ggx_samples", PROPERTY_HINT_RANGE, "0,256,1"), 32);
  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/reflections/sky_reflections/ggx_samples.mobile", PROPERTY_HINT_RANGE, "0,128,1"), 16);
  GLOBAL_DEF("rendering/reflections/sky_reflections/fast_filter_high_quality", false);
  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/reflections/reflection_atlas/reflection_size", PROPERTY_HINT_RANGE, "0,4096,1"), 256);
  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/reflections/reflection_atlas/reflection_size.mobile", PROPERTY_HINT_RANGE, "0,2048,1"), 128);
  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/reflections/reflection_atlas/reflection_count", PROPERTY_HINT_RANGE, "0,256,1"), 64);

  GLOBAL_DEF("rendering/global_illumination/gi/use_half_resolution", false);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/global_illumination/voxel_gi/quality", PROPERTY_HINT_ENUM, "Low (4 Cones - Fast),High (6 Cones - Slow)"), 0);

  GLOBAL_DEF("rendering/shading/overrides/force_vertex_shading", false);
  GLOBAL_DEF("rendering/shading/overrides/force_vertex_shading.mobile", true);
  GLOBAL_DEF("rendering/shading/overrides/force_lambert_over_burley", false);
  GLOBAL_DEF("rendering/shading/overrides/force_lambert_over_burley.mobile", true);

  GLOBAL_DEF_RST("rendering/driver/depth_prepass/enable", true);
  GLOBAL_DEF_RST("rendering/driver/depth_prepass/disable_for_vendors", "PowerVR,Mali,Adreno,Apple");

  GLOBAL_DEF_RST("rendering/textures/default_filters/use_nearest_mipmap_filter", false);
  GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/textures/default_filters/anisotropic_filtering_level", PROPERTY_HINT_ENUM,
                              String::utf8("Disabled (Fastest),2x (Faster),4x (Fast),8x (Average),16x (Slow)")),
                 2);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/camera/depth_of_field/depth_of_field_bokeh_shape", PROPERTY_HINT_ENUM, "Box (Fast),Hexagon (Average),Circle (Slowest)"), 1);
  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/camera/depth_of_field/depth_of_field_bokeh_quality", PROPERTY_HINT_ENUM,
                          "Very Low (Fastest),Low (Fast),Medium (Average),High (Slow)"),
             1);
  GLOBAL_DEF("rendering/camera/depth_of_field/depth_of_field_use_jitter", false);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/environment/ssao/quality", PROPERTY_HINT_ENUM, "Very Low (Fast),Low (Fast),Medium (Average),High (Slow),Ultra (Custom)"),
             2);
  GLOBAL_DEF("rendering/environment/ssao/half_size", true);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/environment/ssao/adaptive_target", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), 0.5);
  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/environment/ssao/blur_passes", PROPERTY_HINT_RANGE, "0,6"), 2);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/environment/ssao/fadeout_from", PROPERTY_HINT_RANGE, "0.0,512,0.1,or_greater"), 50.0);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/environment/ssao/fadeout_to", PROPERTY_HINT_RANGE, "64,65536,0.1,or_greater"), 300.0);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/environment/ssil/quality", PROPERTY_HINT_ENUM, "Very Low (Fast),Low (Fast),Medium (Average),High (Slow),Ultra (Custom)"),
             2);
  GLOBAL_DEF("rendering/environment/ssil/half_size", true);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/environment/ssil/adaptive_target", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), 0.5);
  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/environment/ssil/blur_passes", PROPERTY_HINT_RANGE, "0,6"), 4);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/environment/ssil/fadeout_from", PROPERTY_HINT_RANGE, "0.0,512,0.1,or_greater"), 50.0);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/environment/ssil/fadeout_to", PROPERTY_HINT_RANGE, "64,65536,0.1,or_greater"), 300.0);

  GLOBAL_DEF("rendering/anti_aliasing/screen_space_roughness_limiter/enabled", true);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/anti_aliasing/screen_space_roughness_limiter/amount", PROPERTY_HINT_RANGE, "0.01,4.0,0.01"), 0.25);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/anti_aliasing/screen_space_roughness_limiter/limit", PROPERTY_HINT_RANGE, "0.01,1.0,0.01"), 0.18);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/scaling_3d/mode", PROPERTY_HINT_ENUM, "Bilinear (Fastest),FSR 1.0 (Fast),FSR 2.2 (Slow)"), 0);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/scaling_3d/scale", PROPERTY_HINT_RANGE, "0.25,2.0,0.01"), 1.0);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/scaling_3d/fsr_sharpness", PROPERTY_HINT_RANGE, "0,2,0.1"), 0.2f);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/textures/default_filters/texture_mipmap_bias", PROPERTY_HINT_RANGE, "-2,2,0.001"), 0.0f);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/textures/decals/filter", PROPERTY_HINT_ENUM,
                          "Nearest (Fast),Linear (Fast),Nearest Mipmap (Fast),Linear Mipmap (Fast),Nearest Mipmap Anisotropic (Average),Linear Mipmap Anisotropic (Average)"),
             DECAL_FILTER_LINEAR_MIPMAPS);
  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/textures/light_projectors/filter", PROPERTY_HINT_ENUM,
                          "Nearest (Fast),Linear (Fast),Nearest Mipmap (Fast),Linear Mipmap (Fast),Nearest Mipmap Anisotropic (Average),Linear Mipmap Anisotropic (Average)"),
             LIGHT_PROJECTOR_FILTER_LINEAR_MIPMAPS);

  GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/occlusion_culling/occlusion_rays_per_thread", PROPERTY_HINT_RANGE, "1,2048,1,or_greater"), 512);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/environment/glow/upscale_mode", PROPERTY_HINT_ENUM, "Linear (Fast),Bicubic (Slow)"), 1);
  GLOBAL_DEF("rendering/environment/glow/upscale_mode.mobile", 0);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/environment/screen_space_reflection/roughness_quality", PROPERTY_HINT_ENUM,
                          "Disabled (Fastest),Low (Fast),Medium (Average),High (Slow)"),
             1);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/environment/subsurface_scattering/subsurface_scattering_quality", PROPERTY_HINT_ENUM,
                          "Disabled (Fastest),Low (Fast),Medium (Average),High (Slow)"),
             1);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/environment/subsurface_scattering/subsurface_scattering_scale", PROPERTY_HINT_RANGE, "0.001,1,0.001"), 0.05);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/environment/subsurface_scattering/subsurface_scattering_depth_scale", PROPERTY_HINT_RANGE, "0.001,1,0.001"), 0.01);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/limits/global_shader_variables/buffer_size", PROPERTY_HINT_RANGE, "16,1048576,1"), 65536);

  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/lightmapping/probe_capture/update_speed", PROPERTY_HINT_RANGE, "0.001,256,0.001"), 15);
  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/lightmapping/primitive_meshes/texel_size", PROPERTY_HINT_RANGE, "0.001,100,0.001"), 0.2);
  GLOBAL_DEF("rendering/lightmapping/lightmap_gi/use_bicubic_filter", true);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/global_illumination/sdfgi/probe_ray_count", PROPERTY_HINT_ENUM, "8 (Fastest),16,32,64,96,128 (Slowest)"), 1);
  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/global_illumination/sdfgi/frames_to_converge", PROPERTY_HINT_ENUM,
                          "5 (Less Latency but Lower Quality),10,15,20,25,30 (More Latency but Higher Quality)"),
             5);
  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/global_illumination/sdfgi/frames_to_update_lights", PROPERTY_HINT_ENUM, "1 (Slower),2,4,8,16 (Faster)"), 2);

  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/environment/volumetric_fog/volume_size", PROPERTY_HINT_RANGE, "16,512,1"), 64);
  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/environment/volumetric_fog/volume_depth", PROPERTY_HINT_RANGE, "16,512,1"), 64);
  GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/environment/volumetric_fog/use_filter", PROPERTY_HINT_ENUM, "No (Faster),Yes (Higher Quality)"), 1);

  GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/limits/spatial_indexer/update_iterations_per_frame", PROPERTY_HINT_RANGE, "0,1024,1"), 10);
  GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/limits/spatial_indexer/threaded_cull_minimum_instances", PROPERTY_HINT_RANGE, "32,65536,1"), 1000);

  GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/limits/cluster_builder/max_clustered_elements", PROPERTY_HINT_RANGE, "32,8192,1"), 512);

  // OpenGL limits
  GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/limits/opengl/max_renderable_elements", PROPERTY_HINT_RANGE, "1024,65536,1"), 65536);
  GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/limits/opengl/max_renderable_lights", PROPERTY_HINT_RANGE, "2,256,1"), 32);
  GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/limits/opengl/max_lights_per_object", PROPERTY_HINT_RANGE, "2,1024,1"), 8);

//   GLOBAL_DEF_RST_BASIC("xr/shaders/enabled", false);

  GLOBAL_DEF("debug/shader_language/warnings/enable", true);
  GLOBAL_DEF("debug/shader_language/warnings/treat_warnings_as_errors", false);

// #ifdef DEBUG_ENABLED
//   for (int i = 0; i < (int)ShaderWarning::WARNING_MAX; i++) {
//     GLOBAL_DEF("debug/shader_language/warnings/" + ShaderWarning::get_name_from_code((ShaderWarning::Code)i).to_lower(), true);
//   }
// #endif
}

