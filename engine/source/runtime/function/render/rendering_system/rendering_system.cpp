#include "rendering_system.h"
#include "function/render/renderer_rd/storage/mesh_storage.h"
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


void RS::mesh_add_surface_from_arrays(RID p_mesh, PrimitiveType p_primitive, const Array &p_arrays, const Array &p_blend_shapes, const Dictionary &p_lods, BitField<ArrayFormat> p_compress_format) {
	SurfaceData sd;
	Error err = mesh_create_surface_data_from_arrays(&sd, p_primitive, p_arrays, p_blend_shapes, p_lods, p_compress_format);
	if (err != OK) {
		return;
	}
	mesh_add_surface(p_mesh, sd);
}

void RS::mesh_surface_make_offsets_from_format(uint64_t p_format, int p_vertex_len, int p_index_len, uint32_t *r_offsets, uint32_t &r_vertex_element_size, uint32_t &r_normal_element_size, uint32_t &r_attrib_element_size, uint32_t &r_skin_element_size) const {
	r_vertex_element_size = 0;
	r_normal_element_size = 0;
	r_attrib_element_size = 0;
	r_skin_element_size = 0;

	uint32_t *size_accum = nullptr;

	for (int i = 0; i < RS::ARRAY_MAX; i++) {
		r_offsets[i] = 0; // Reset

		if (i == RS::ARRAY_VERTEX) {
			size_accum = &r_vertex_element_size;
		} else if (i == RS::ARRAY_NORMAL) {
			size_accum = &r_normal_element_size;
		} else if (i == RS::ARRAY_COLOR) {
			size_accum = &r_attrib_element_size;
		} else if (i == RS::ARRAY_BONES) {
			size_accum = &r_skin_element_size;
		}

		if (!(p_format & (1ULL << i))) { // No array
			continue;
		}

		int elem_size = 0;

		switch (i) {
			case RS::ARRAY_VERTEX: {
				if (p_format & ARRAY_FLAG_USE_2D_VERTICES) {
					elem_size = 2;
				} else {
					elem_size = (p_format & ARRAY_FLAG_COMPRESS_ATTRIBUTES) ? 2 : 3;
				}

				elem_size *= sizeof(float);
			} break;
			case RS::ARRAY_NORMAL: {
				elem_size = 4;
			} break;
			case RS::ARRAY_TANGENT: {
				elem_size = (p_format & ARRAY_FLAG_COMPRESS_ATTRIBUTES) ? 0 : 4;
			} break;
			case RS::ARRAY_COLOR: {
				elem_size = 4;
			} break;
			case RS::ARRAY_TEX_UV: {
				elem_size = (p_format & ARRAY_FLAG_COMPRESS_ATTRIBUTES) ? 4 : 8;
			} break;
			case RS::ARRAY_TEX_UV2: {
				elem_size = (p_format & ARRAY_FLAG_COMPRESS_ATTRIBUTES) ? 4 : 8;
			} break;
			case RS::ARRAY_CUSTOM0:
			case RS::ARRAY_CUSTOM1:
			case RS::ARRAY_CUSTOM2:
			case RS::ARRAY_CUSTOM3: {
				uint64_t format = (p_format >> (ARRAY_FORMAT_CUSTOM_BASE + (ARRAY_FORMAT_CUSTOM_BITS * (i - ARRAY_CUSTOM0)))) & ARRAY_FORMAT_CUSTOM_MASK;
				switch (format) {
					case ARRAY_CUSTOM_RGBA8_UNORM: {
						elem_size = 4;
					} break;
					case ARRAY_CUSTOM_RGBA8_SNORM: {
						elem_size = 4;
					} break;
					case ARRAY_CUSTOM_RG_HALF: {
						elem_size = 4;
					} break;
					case ARRAY_CUSTOM_RGBA_HALF: {
						elem_size = 8;
					} break;
					case ARRAY_CUSTOM_R_FLOAT: {
						elem_size = 4;
					} break;
					case ARRAY_CUSTOM_RG_FLOAT: {
						elem_size = 8;
					} break;
					case ARRAY_CUSTOM_RGB_FLOAT: {
						elem_size = 12;
					} break;
					case ARRAY_CUSTOM_RGBA_FLOAT: {
						elem_size = 16;
					} break;
				}
			} break;
			case RS::ARRAY_WEIGHTS: {
				uint32_t bone_count = (p_format & ARRAY_FLAG_USE_8_BONE_WEIGHTS) ? 8 : 4;
				elem_size = sizeof(uint16_t) * bone_count;

			} break;
			case RS::ARRAY_BONES: {
				uint32_t bone_count = (p_format & ARRAY_FLAG_USE_8_BONE_WEIGHTS) ? 8 : 4;
				elem_size = sizeof(uint16_t) * bone_count;
			} break;
			case RS::ARRAY_INDEX: {
				if (p_index_len <= 0) {
					ERR_PRINT("index_array_len==NO_INDEX_ARRAY");
					break;
				}
				/* determine whether using 16 or 32 bits indices */
				if (p_vertex_len <= (1 << 16) && p_vertex_len > 0) {
					elem_size = 2;
				} else {
					elem_size = 4;
				}
				r_offsets[i] = elem_size;
				continue;
			}
			default: {
				ERR_FAIL();
			}
		}

		if (size_accum != nullptr) {
			r_offsets[i] = (*size_accum);
			if (i == RS::ARRAY_NORMAL || i == RS::ARRAY_TANGENT) {
				r_offsets[i] += r_vertex_element_size * p_vertex_len;
			}
			(*size_accum) += elem_size;
		} else {
			r_offsets[i] = 0;
		}
	}
}

Error lain::RenderingSystem::mesh_create_surface_data_from_arrays(SurfaceData* r_surface_data, PrimitiveType p_primitive, const Array& p_arrays, const Array& p_blend_shapes,
                                                                  const Dictionary& p_lods, uint64_t p_compress_format) {
  ERR_FAIL_INDEX_V(p_primitive, RS::PRIMITIVE_MAX, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_arrays.size() != RS::ARRAY_MAX, ERR_INVALID_PARAMETER);

	uint64_t format = 0;

	// Validation
	int index_array_len = 0;
	int array_len = 0;

	for (int i = 0; i < p_arrays.size(); i++) {
		if (p_arrays[i].get_type() == Variant::NIL) {
			continue;
		}

		format |= (1ULL << i);

		if (i == RS::ARRAY_VERTEX) {
			switch (p_arrays[i].get_type()) {
				case Variant::PACKED_VECTOR2_ARRAY: {
					Vector<Vector2> v2 = p_arrays[i];
					array_len = v2.size();
					format |= ARRAY_FLAG_USE_2D_VERTICES;
				} break;
				case Variant::PACKED_VECTOR3_ARRAY: {
					ERR_FAIL_COND_V(p_compress_format & ARRAY_FLAG_USE_2D_VERTICES, ERR_INVALID_PARAMETER);
					Vector<Vector3> v3 = p_arrays[i];
					array_len = v3.size();
				} break;
				default: {
					ERR_FAIL_V_MSG(ERR_INVALID_DATA, "Vertex array must be a PackedVector2Array or PackedVector3Array.");
				} break;
			}
			ERR_FAIL_COND_V(array_len == 0, ERR_INVALID_DATA);
		} else if (i == RS::ARRAY_NORMAL) {
			if (p_arrays[RS::ARRAY_TANGENT].get_type() == Variant::NIL) {
				// We must use tangents if using normals.
				format |= (1ULL << RS::ARRAY_TANGENT);
			}
		} else if (i == RS::ARRAY_BONES) {
			switch (p_arrays[i].get_type()) {
				case Variant::PACKED_INT32_ARRAY: {
					Vector<Vector3> vertices = p_arrays[RS::ARRAY_VERTEX];
					Vector<int32_t> bones = p_arrays[i];
					int32_t bone_8_group_count = bones.size() / (ARRAY_WEIGHTS_SIZE * 2);
					int32_t vertex_count = vertices.size();
					if (vertex_count == bone_8_group_count) {
						format |= RS::ARRAY_FLAG_USE_8_BONE_WEIGHTS;
					}
				} break;
				default: {
					ERR_FAIL_V_MSG(ERR_INVALID_DATA, "Bones array must be a PackedInt32Array.");
				} break;
			}
		} else if (i == RS::ARRAY_INDEX) {
			index_array_len = PackedInt32Array(p_arrays[i]).size();
		}
	}

	if (p_blend_shapes.size()) {
		// Validate format for morphs.
		for (int i = 0; i < p_blend_shapes.size(); i++) {
			uint32_t bsformat = 0;
			Array arr = p_blend_shapes[i];
			for (int j = 0; j < arr.size(); j++) {
				if (arr[j].get_type() != Variant::NIL) {
					bsformat |= (1 << j);
				}
			}
			if (bsformat & RS::ARRAY_FORMAT_NORMAL) {
				// We must use tangents if using normals.
				bsformat |= RS::ARRAY_FORMAT_TANGENT;
			}

			ERR_FAIL_COND_V_MSG(bsformat != (format & RS::ARRAY_FORMAT_BLEND_SHAPE_MASK), ERR_INVALID_PARAMETER, "Blend shape format must match the main array format for Vertex, Normal and Tangent arrays.");
		}
	}

	for (uint32_t i = 0; i < RS::ARRAY_CUSTOM_COUNT; ++i) {
		// Include custom array format type.
		if (format & (1ULL << (ARRAY_CUSTOM0 + i))) {
			format |= (RS::ARRAY_FORMAT_CUSTOM_MASK << (RS::ARRAY_FORMAT_CUSTOM_BASE + i * RS::ARRAY_FORMAT_CUSTOM_BITS)) & p_compress_format;
		}
	}

	uint32_t offsets[RS::ARRAY_MAX];

	uint32_t vertex_element_size;
	uint32_t normal_element_size;
	uint32_t attrib_element_size;
	uint32_t skin_element_size;

	uint64_t mask = (1ULL << ARRAY_MAX) - 1ULL;
	format |= (~mask) & p_compress_format; // Make the full format.

	// Force version to the current version as this function will always return a surface with the current version.
	format &= ~(ARRAY_FLAG_FORMAT_VERSION_MASK << ARRAY_FLAG_FORMAT_VERSION_SHIFT);
	format |= ARRAY_FLAG_FORMAT_CURRENT_VERSION & (ARRAY_FLAG_FORMAT_VERSION_MASK << ARRAY_FLAG_FORMAT_VERSION_SHIFT);

	mesh_surface_make_offsets_from_format(format, array_len, index_array_len, offsets, vertex_element_size, normal_element_size, attrib_element_size, skin_element_size);

	if ((format & RS::ARRAY_FORMAT_VERTEX) == 0 && !(format & RS::ARRAY_FLAG_USES_EMPTY_VERTEX_ARRAY)) {
		ERR_PRINT("Mesh created without vertex array. This mesh will not be visible with the default shader. If using an empty vertex array is intentional, create the mesh with the ARRAY_FLAG_USES_EMPTY_VERTEX_ARRAY flag to silence this error.");
		// Set the flag here after warning to suppress errors down the pipeline.
		format |= RS::ARRAY_FLAG_USES_EMPTY_VERTEX_ARRAY;
	}

	if ((format & RS::ARRAY_FLAG_COMPRESS_ATTRIBUTES) && ((format & RS::ARRAY_FORMAT_NORMAL) || (format & RS::ARRAY_FORMAT_TANGENT))) {
		// If using normals or tangents, then we need all three.
		ERR_FAIL_COND_V_MSG(!(format & RS::ARRAY_FORMAT_VERTEX), ERR_INVALID_PARAMETER, "Can't use compression flag 'ARRAY_FLAG_COMPRESS_ATTRIBUTES' while using normals or tangents without vertex array.");
		ERR_FAIL_COND_V_MSG(!(format & RS::ARRAY_FORMAT_NORMAL), ERR_INVALID_PARAMETER, "Can't use compression flag 'ARRAY_FLAG_COMPRESS_ATTRIBUTES' while using tangents without normal array.");
	}

	int vertex_array_size = (vertex_element_size + normal_element_size) * array_len;
	int attrib_array_size = attrib_element_size * array_len;
	int skin_array_size = skin_element_size * array_len;
	int index_array_size = offsets[RS::ARRAY_INDEX] * index_array_len;

	Vector<uint8_t> vertex_array;
	vertex_array.resize(vertex_array_size);

	Vector<uint8_t> attrib_array;
	attrib_array.resize(attrib_array_size);

	Vector<uint8_t> skin_array;
	skin_array.resize(skin_array_size);

	Vector<uint8_t> index_array;
	index_array.resize(index_array_size);

	AABB aabb;
	Vector<AABB> bone_aabb;

	Vector4 uv_scale = Vector4(0.0, 0.0, 0.0, 0.0);

	Error err = _surface_set_data(p_arrays, format, offsets, vertex_element_size, normal_element_size, attrib_element_size, skin_element_size, vertex_array, attrib_array, skin_array, array_len, index_array, index_array_len, aabb, bone_aabb, uv_scale);
	ERR_FAIL_COND_V_MSG(err != OK, ERR_INVALID_DATA, "Invalid array format for surface.");

	Vector<uint8_t> blend_shape_data;

	if (p_blend_shapes.size()) {
		uint32_t bs_format = format & RS::ARRAY_FORMAT_BLEND_SHAPE_MASK;
		for (int i = 0; i < p_blend_shapes.size(); i++) {
			Vector<uint8_t> vertex_array_shape;
			vertex_array_shape.resize(vertex_array_size);
			Vector<uint8_t> noindex;
			Vector<uint8_t> noattrib;
			Vector<uint8_t> noskin;

			AABB laabb;
			Vector4 bone_uv_scale; // Not used.
			Error err2 = _surface_set_data(p_blend_shapes[i], bs_format, offsets, vertex_element_size, normal_element_size, 0, 0, vertex_array_shape, noattrib, noskin, array_len, noindex, 0, laabb, bone_aabb, bone_uv_scale);
			aabb.merge_with(laabb);
			ERR_FAIL_COND_V_MSG(err2 != OK, ERR_INVALID_DATA, "Invalid blend shape array format for surface.");

			blend_shape_data.append_array(vertex_array_shape);
		}
	}
	Vector<SurfaceData::LOD> lods;
	if (index_array_len) {
		List<Variant> keys;
		p_lods.get_key_list(&keys);
		keys.sort(); // otherwise lod levels may get skipped
		for (const Variant &E : keys) {
			float distance = E;
			ERR_CONTINUE(distance <= 0.0);
			Vector<int> indices = p_lods[E];
			ERR_CONTINUE(indices.is_empty());
			uint32_t index_count = indices.size();
			ERR_CONTINUE(index_count >= (uint32_t)index_array_len); // Should be smaller..

			const int *r = indices.ptr();

			Vector<uint8_t> data;
			if (array_len <= 65536) {
				// 16 bits indices
				data.resize(indices.size() * 2);
				uint8_t *w = data.ptrw();
				uint16_t *index_ptr = (uint16_t *)w;
				for (uint32_t i = 0; i < index_count; i++) {
					index_ptr[i] = r[i];
				}
			} else {
				// 32 bits indices
				data.resize(indices.size() * 4);
				uint8_t *w = data.ptrw();
				uint32_t *index_ptr = (uint32_t *)w;
				for (uint32_t i = 0; i < index_count; i++) {
					index_ptr[i] = r[i];
				}
			}

			SurfaceData::LOD lod;
			lod.edge_length = distance;
			lod.index_data = data;
			lods.push_back(lod);
		}
	}

	SurfaceData &surface_data = *r_surface_data;
	surface_data.format = format;
	surface_data.primitive = p_primitive;
	surface_data.aabb = aabb;
	surface_data.vertex_data = vertex_array;
	surface_data.attribute_data = attrib_array;
	surface_data.skin_data = skin_array;
	surface_data.vertex_count = array_len;
	surface_data.index_data = index_array;
	surface_data.index_count = index_array_len;
	surface_data.blend_shape_data = blend_shape_data;
	surface_data.bone_aabbs = bone_aabb;
	surface_data.lods = lods;
	surface_data.uv_scale = uv_scale;

	return OK;
}





AABB _compute_aabb_from_points(const Vector3 *p_data, int p_length) {
	if (p_length == 0) {
		return AABB();
	}

	Vector3 min = p_data[0];
	Vector3 max = p_data[0];

	for (int i = 1; i < p_length; ++i) {
		min = min.min(p_data[i]);
		max = max.max(p_data[i]);
	}

	return AABB(min, max - min);
}

void _get_axis_angle(const Vector3 &p_normal, const Vector4 &p_tangent, float &r_angle, Vector3 &r_axis) {
	Vector3 normal = p_normal.normalized();
	Vector3 tangent = Vector3(p_tangent.x, p_tangent.y, p_tangent.z).normalized();
	float d = p_tangent.w;
	Vector3 binormal = normal.cross(tangent).normalized();
	real_t angle;

	Basis tbn = Basis();
	tbn.rows[0] = tangent;
	tbn.rows[1] = binormal;
	tbn.rows[2] = normal;
	tbn.get_axis_angle(r_axis, angle);
	r_angle = float(angle);

	if (d < 0.0) {
		r_angle = CLAMP((1.0 - r_angle / Math_PI) * 0.5, 0.0, 0.49999);
	} else {
		r_angle = CLAMP((r_angle / Math_PI) * 0.5 + 0.5, 0.500008, 1.0);
	}
}

Error RS::_surface_set_data(Array p_arrays, uint64_t p_format, uint32_t *p_offsets, uint32_t p_vertex_stride, uint32_t p_normal_stride, uint32_t p_attrib_stride, uint32_t p_skin_stride, Vector<uint8_t> &r_vertex_array, Vector<uint8_t> &r_attrib_array, Vector<uint8_t> &r_skin_array, int p_vertex_array_len, Vector<uint8_t> &r_index_array, int p_index_array_len, AABB &r_aabb, Vector<AABB> &r_bone_aabb, Vector4 &r_uv_scale) {
	uint8_t *vw = r_vertex_array.ptrw();
	uint8_t *aw = r_attrib_array.ptrw();
	uint8_t *sw = r_skin_array.ptrw();

	uint8_t *iw = nullptr;
	if (r_index_array.size()) {
		iw = r_index_array.ptrw();
	}

	int max_bone = 0;

	// Preprocess UVs if compression is enabled
	if (p_format & RS::ARRAY_FLAG_COMPRESS_ATTRIBUTES && ((p_format & RS::ARRAY_FORMAT_TEX_UV) || (p_format & RS::ARRAY_FORMAT_TEX_UV2))) {
		const Vector2 *uv_src = nullptr;
		if (p_format & RS::ARRAY_FORMAT_TEX_UV) {
			Vector<Vector2> array = p_arrays[RS::ARRAY_TEX_UV];
			uv_src = array.ptr();
		}

		const Vector2 *uv2_src = nullptr;
		if (p_format & RS::ARRAY_FORMAT_TEX_UV2) {
			Vector<Vector2> array = p_arrays[RS::ARRAY_TEX_UV2];
			uv2_src = array.ptr();
		}

		Vector2 max_val = Vector2(0.0, 0.0);
		Vector2 min_val = Vector2(0.0, 0.0);
		Vector2 max_val2 = Vector2(0.0, 0.0);
		Vector2 min_val2 = Vector2(0.0, 0.0);

		for (int i = 0; i < p_vertex_array_len; i++) {
			if (p_format & RS::ARRAY_FORMAT_TEX_UV) {
				max_val = max_val.max(uv_src[i]);
				min_val = min_val.min(uv_src[i]);
			}
			if (p_format & RS::ARRAY_FORMAT_TEX_UV2) {
				max_val2 = max_val2.max(uv2_src[i]);
				min_val2 = min_val2.min(uv2_src[i]);
			}
		}

		max_val = max_val.abs().max(min_val.abs());
		max_val2 = max_val2.abs().max(min_val2.abs());

		if (min_val.x >= 0.0 && min_val2.x >= 0.0 && max_val.x <= 1.0 && max_val2.x <= 1.0 &&
				min_val.y >= 0.0 && min_val2.y >= 0.0 && max_val.y <= 1.0 && max_val2.y <= 1.0) {
			// When all channels are in the 0-1 range, we will compress to 16-bit without scaling to
			// preserve the bits as best as possible.
			r_uv_scale = Vector4(0.0, 0.0, 0.0, 0.0);
		} else {
			r_uv_scale = Vector4(max_val.x, max_val.y, max_val2.x, max_val2.y) * Vector4(2.0, 2.0, 2.0, 2.0);
		}
	}

	for (int ai = 0; ai < RS::ARRAY_MAX; ai++) {
		if (!(p_format & (1ULL << ai))) { // No array
			continue;
		}

		switch (ai) {
			case RS::ARRAY_VERTEX: {
				if (p_format & RS::ARRAY_FLAG_USE_2D_VERTICES) {
					Vector<Vector2> array = p_arrays[ai];
					ERR_FAIL_COND_V(array.size() != p_vertex_array_len, ERR_INVALID_PARAMETER);

					const Vector2 *src = array.ptr();

					// Setting vertices means regenerating the AABB.
					Rect2 aabb;

					{
						for (int i = 0; i < p_vertex_array_len; i++) {
							float vector[2] = { (float)src[i].x, (float)src[i].y };

							memcpy(&vw[p_offsets[ai] + i * p_vertex_stride], vector, sizeof(float) * 2);

							if (i == 0) {
								aabb = Rect2(src[i], SMALL_VEC2); // Must have a bit of size.
							} else {
								aabb.expand_to(src[i]);
							}
						}
					}

					r_aabb = AABB(Vector3(aabb.position.x, aabb.position.y, 0), Vector3(aabb.size.x, aabb.size.y, 0));

				} else {
					Vector<Vector3> array = p_arrays[ai];
					ERR_FAIL_COND_V(array.size() != p_vertex_array_len, ERR_INVALID_PARAMETER);

					const Vector3 *src = array.ptr();

					r_aabb = _compute_aabb_from_points(src, p_vertex_array_len);
					r_aabb.size = r_aabb.size.max(SMALL_VEC3);

					if (p_format & ARRAY_FLAG_COMPRESS_ATTRIBUTES) {
						if (!(p_format & RS::ARRAY_FORMAT_NORMAL)) {
							// Early out if we are only setting vertex positions.
							for (int i = 0; i < p_vertex_array_len; i++) {
								Vector3 pos = (src[i] - r_aabb.position) / r_aabb.size;
								uint16_t vector[4] = {
									(uint16_t)CLAMP(pos.x * 65535, 0, 65535),
									(uint16_t)CLAMP(pos.y * 65535, 0, 65535),
									(uint16_t)CLAMP(pos.z * 65535, 0, 65535),
									(uint16_t)0
								};

								memcpy(&vw[p_offsets[ai] + i * p_vertex_stride], vector, sizeof(uint16_t) * 4);
							}
							continue;
						}

						// Validate normal and tangent arrays.
						ERR_FAIL_COND_V(p_arrays[RS::ARRAY_NORMAL].get_type() != Variant::PACKED_VECTOR3_ARRAY, ERR_INVALID_PARAMETER);

						Vector<Vector3> normal_array = p_arrays[RS::ARRAY_NORMAL];
						ERR_FAIL_COND_V(normal_array.size() != p_vertex_array_len, ERR_INVALID_PARAMETER);
						const Vector3 *normal_src = normal_array.ptr();

						Variant::Type tangent_type = p_arrays[RS::ARRAY_TANGENT].get_type();
						ERR_FAIL_COND_V(tangent_type != Variant::PACKED_FLOAT32_ARRAY && tangent_type != Variant::PACKED_FLOAT64_ARRAY && tangent_type != Variant::NIL, ERR_INVALID_PARAMETER);

						// We need a different version if using double precision tangents.
						if (tangent_type == Variant::PACKED_FLOAT32_ARRAY) {
							Vector<float> tangent_array = p_arrays[RS::ARRAY_TANGENT];
							ERR_FAIL_COND_V(tangent_array.size() != p_vertex_array_len * 4, ERR_INVALID_PARAMETER);
							const float *tangent_src = tangent_array.ptr();

							// Set data for vertex, normal, and tangent.
							for (int i = 0; i < p_vertex_array_len; i++) {
								float angle = 0.0;
								Vector3 axis;
								Vector4 tangent = Vector4(tangent_src[i * 4 + 0], tangent_src[i * 4 + 1], tangent_src[i * 4 + 2], tangent_src[i * 4 + 3]);
								_get_axis_angle(normal_src[i], tangent, angle, axis);

								// Store axis.
								{
									Vector2 res = axis.octahedron_encode();
									uint16_t vector[2] = {
										(uint16_t)CLAMP(res.x * 65535, 0, 65535),
										(uint16_t)CLAMP(res.y * 65535, 0, 65535),
									};

									memcpy(&vw[p_offsets[RS::ARRAY_NORMAL] + i * p_normal_stride], vector, 4);
								}

								// Store vertex position + angle.
								{
									Vector3 pos = (src[i] - r_aabb.position) / r_aabb.size;
									uint16_t vector[4] = {
										(uint16_t)CLAMP(pos.x * 65535, 0, 65535),
										(uint16_t)CLAMP(pos.y * 65535, 0, 65535),
										(uint16_t)CLAMP(pos.z * 65535, 0, 65535),
										(uint16_t)CLAMP(angle * 65535, 0, 65535)
									};

									memcpy(&vw[p_offsets[ai] + i * p_vertex_stride], vector, sizeof(uint16_t) * 4);
								}
							}
						} else if (tangent_type == Variant::PACKED_FLOAT64_ARRAY) {
							Vector<double> tangent_array = p_arrays[RS::ARRAY_TANGENT];
							ERR_FAIL_COND_V(tangent_array.size() != p_vertex_array_len * 4, ERR_INVALID_PARAMETER);
							const double *tangent_src = tangent_array.ptr();

							// Set data for vertex, normal, and tangent.
							for (int i = 0; i < p_vertex_array_len; i++) {
								float angle;
								Vector3 axis;
								Vector4 tangent = Vector4(tangent_src[i * 4 + 0], tangent_src[i * 4 + 1], tangent_src[i * 4 + 2], tangent_src[i * 4 + 3]);
								_get_axis_angle(normal_src[i], tangent, angle, axis);

								// Store axis.
								{
									Vector2 res = axis.octahedron_encode();
									uint16_t vector[2] = {
										(uint16_t)CLAMP(res.x * 65535, 0, 65535),
										(uint16_t)CLAMP(res.y * 65535, 0, 65535),
									};

									memcpy(&vw[p_offsets[RS::ARRAY_NORMAL] + i * p_normal_stride], vector, 4);
								}

								// Store vertex position + angle.
								{
									Vector3 pos = (src[i] - r_aabb.position) / r_aabb.size;
									uint16_t vector[4] = {
										(uint16_t)CLAMP(pos.x * 65535, 0, 65535),
										(uint16_t)CLAMP(pos.y * 65535, 0, 65535),
										(uint16_t)CLAMP(pos.z * 65535, 0, 65535),
										(uint16_t)CLAMP(angle * 65535, 0, 65535)
									};

									memcpy(&vw[p_offsets[ai] + i * p_vertex_stride], vector, sizeof(uint16_t) * 4);
								}
							}
						} else { // No tangent array.
							// Set data for vertex, normal, and tangent.
							for (int i = 0; i < p_vertex_array_len; i++) {
								float angle;
								Vector3 axis;
								// Generate an arbitrary vector that is tangential to normal.
								// This assumes that the normal is never (0,0,0).
								Vector3 tan = Vector3(normal_src[i].z, -normal_src[i].x, normal_src[i].y).cross(normal_src[i].normalized()).normalized();
								Vector4 tangent = Vector4(tan.x, tan.y, tan.z, 1.0);
								_get_axis_angle(normal_src[i], tangent, angle, axis);

								// Store axis.
								{
									Vector2 res = axis.octahedron_encode();
									uint16_t vector[2] = {
										(uint16_t)CLAMP(res.x * 65535, 0, 65535),
										(uint16_t)CLAMP(res.y * 65535, 0, 65535),
									};

									memcpy(&vw[p_offsets[RS::ARRAY_NORMAL] + i * p_normal_stride], vector, 4);
								}

								// Store vertex position + angle.
								{
									Vector3 pos = (src[i] - r_aabb.position) / r_aabb.size;
									uint16_t vector[4] = {
										(uint16_t)CLAMP(pos.x * 65535, 0, 65535),
										(uint16_t)CLAMP(pos.y * 65535, 0, 65535),
										(uint16_t)CLAMP(pos.z * 65535, 0, 65535),
										(uint16_t)CLAMP(angle * 65535, 0, 65535)
									};

									memcpy(&vw[p_offsets[ai] + i * p_vertex_stride], vector, sizeof(uint16_t) * 4);
								}
							}
						}
					} else {
						for (int i = 0; i < p_vertex_array_len; i++) {
							float vector[3] = { (float)src[i].x, (float)src[i].y, (float)src[i].z };

							memcpy(&vw[p_offsets[ai] + i * p_vertex_stride], vector, sizeof(float) * 3);
						}
					}
				}

			} break;
			case RS::ARRAY_NORMAL: {
				// If using compression we store normal while storing vertices.
				if (!(p_format & RS::ARRAY_FLAG_COMPRESS_ATTRIBUTES)) {
					ERR_FAIL_COND_V(p_arrays[ai].get_type() != Variant::PACKED_VECTOR3_ARRAY, ERR_INVALID_PARAMETER);

					Vector<Vector3> array = p_arrays[ai];
					ERR_FAIL_COND_V(array.size() != p_vertex_array_len, ERR_INVALID_PARAMETER);

					const Vector3 *src = array.ptr();
					for (int i = 0; i < p_vertex_array_len; i++) {
						Vector2 res = src[i].octahedron_encode();
						uint16_t vector[2] = {
							(uint16_t)CLAMP(res.x * 65535, 0, 65535),
							(uint16_t)CLAMP(res.y * 65535, 0, 65535),
						};

						memcpy(&vw[p_offsets[ai] + i * p_normal_stride], vector, 4);
					}
				}
			} break;

			case RS::ARRAY_TANGENT: {
				// If using compression we store tangent while storing vertices.
				if (!(p_format & RS::ARRAY_FLAG_COMPRESS_ATTRIBUTES)) {
					Variant::Type type = p_arrays[ai].get_type();
					ERR_FAIL_COND_V(type != Variant::PACKED_FLOAT32_ARRAY && type != Variant::PACKED_FLOAT64_ARRAY && type != Variant::NIL, ERR_INVALID_PARAMETER);

					if (type == Variant::PACKED_FLOAT32_ARRAY) {
						Vector<float> array = p_arrays[ai];
						ERR_FAIL_COND_V(array.size() != p_vertex_array_len * 4, ERR_INVALID_PARAMETER);
						const float *src_ptr = array.ptr();

						for (int i = 0; i < p_vertex_array_len; i++) {
							const Vector3 src(src_ptr[i * 4 + 0], src_ptr[i * 4 + 1], src_ptr[i * 4 + 2]);
							Vector2 res = src.octahedron_tangent_encode(src_ptr[i * 4 + 3]);
							uint16_t vector[2] = {
								(uint16_t)CLAMP(res.x * 65535, 0, 65535),
								(uint16_t)CLAMP(res.y * 65535, 0, 65535),
							};

							if (vector[0] == 0 && vector[1] == 65535) {
								// (1, 1) and (0, 1) decode to the same value, but (0, 1) messes with our compression detection.
								// So we sanitize here.
								vector[0] = 65535;
							}

							memcpy(&vw[p_offsets[ai] + i * p_normal_stride], vector, 4);
						}
					} else if (type == Variant::PACKED_FLOAT64_ARRAY) {
						Vector<double> array = p_arrays[ai];
						ERR_FAIL_COND_V(array.size() != p_vertex_array_len * 4, ERR_INVALID_PARAMETER);
						const double *src_ptr = array.ptr();

						for (int i = 0; i < p_vertex_array_len; i++) {
							const Vector3 src(src_ptr[i * 4 + 0], src_ptr[i * 4 + 1], src_ptr[i * 4 + 2]);
							Vector2 res = src.octahedron_tangent_encode(src_ptr[i * 4 + 3]);
							uint16_t vector[2] = {
								(uint16_t)CLAMP(res.x * 65535, 0, 65535),
								(uint16_t)CLAMP(res.y * 65535, 0, 65535),
							};

							if (vector[0] == 0 && vector[1] == 65535) {
								// (1, 1) and (0, 1) decode to the same value, but (0, 1) messes with our compression detection.
								// So we sanitize here.
								vector[0] = 65535;
							}

							memcpy(&vw[p_offsets[ai] + i * p_normal_stride], vector, 4);
						}
					} else { // No tangent array.
						ERR_FAIL_COND_V(p_arrays[RS::ARRAY_NORMAL].get_type() != Variant::PACKED_VECTOR3_ARRAY, ERR_INVALID_PARAMETER);

						Vector<Vector3> normal_array = p_arrays[RS::ARRAY_NORMAL];
						ERR_FAIL_COND_V(normal_array.size() != p_vertex_array_len, ERR_INVALID_PARAMETER);
						const Vector3 *normal_src = normal_array.ptr();
						// Set data for tangent.
						for (int i = 0; i < p_vertex_array_len; i++) {
							// Generate an arbitrary vector that is tangential to normal.
							// This assumes that the normal is never (0,0,0).
							Vector3 tan = Vector3(normal_src[i].z, -normal_src[i].x, normal_src[i].y).cross(normal_src[i].normalized()).normalized();
							Vector2 res = tan.octahedron_tangent_encode(1.0);
							uint16_t vector[2] = {
								(uint16_t)CLAMP(res.x * 65535, 0, 65535),
								(uint16_t)CLAMP(res.y * 65535, 0, 65535),
							};

							if (vector[0] == 0 && vector[1] == 65535) {
								// (1, 1) and (0, 1) decode to the same value, but (0, 1) messes with our compression detection.
								// So we sanitize here.
								vector[0] = 65535;
							}

							memcpy(&vw[p_offsets[ai] + i * p_normal_stride], vector, 4);
						}
					}
				}
			} break;
			case RS::ARRAY_COLOR: {
				ERR_FAIL_COND_V(p_arrays[ai].get_type() != Variant::PACKED_COLOR_ARRAY, ERR_INVALID_PARAMETER);

				Vector<Color> array = p_arrays[ai];

				ERR_FAIL_COND_V(array.size() != p_vertex_array_len, ERR_INVALID_PARAMETER);

				const Color *src = array.ptr();
				for (int i = 0; i < p_vertex_array_len; i++) {
					uint8_t color8[4] = {
						uint8_t(CLAMP(src[i].r * 255.0, 0.0, 255.0)),
						uint8_t(CLAMP(src[i].g * 255.0, 0.0, 255.0)),
						uint8_t(CLAMP(src[i].b * 255.0, 0.0, 255.0)),
						uint8_t(CLAMP(src[i].a * 255.0, 0.0, 255.0))
					};
					memcpy(&aw[p_offsets[ai] + i * p_attrib_stride], color8, 4);
				}
			} break;
			case RS::ARRAY_TEX_UV: {
				ERR_FAIL_COND_V(p_arrays[ai].get_type() != Variant::PACKED_VECTOR3_ARRAY && p_arrays[ai].get_type() != Variant::PACKED_VECTOR2_ARRAY, ERR_INVALID_PARAMETER);

				Vector<Vector2> array = p_arrays[ai];

				ERR_FAIL_COND_V(array.size() != p_vertex_array_len, ERR_INVALID_PARAMETER);

				const Vector2 *src = array.ptr();
				if (p_format & RS::ARRAY_FLAG_COMPRESS_ATTRIBUTES) {
					for (int i = 0; i < p_vertex_array_len; i++) {
						Vector2 vec = src[i];
						if (!r_uv_scale.is_zero_approx()) {
							// Normalize into 0-1 from possible range -uv_scale - uv_scale.
							vec = vec / (Vector2(r_uv_scale.x, r_uv_scale.y)) + Vector2(0.5, 0.5);
						}

						uint16_t uv[2] = { (uint16_t)CLAMP(vec.x * 65535, 0, 65535), (uint16_t)CLAMP(vec.y * 65535, 0, 65535) };
						memcpy(&aw[p_offsets[ai] + i * p_attrib_stride], uv, 4);
					}
				} else {
					for (int i = 0; i < p_vertex_array_len; i++) {
						float uv[2] = { (float)src[i].x, (float)src[i].y };
						memcpy(&aw[p_offsets[ai] + i * p_attrib_stride], uv, 2 * 4);
					}
				}
			} break;

			case RS::ARRAY_TEX_UV2: {
				ERR_FAIL_COND_V(p_arrays[ai].get_type() != Variant::PACKED_VECTOR3_ARRAY && p_arrays[ai].get_type() != Variant::PACKED_VECTOR2_ARRAY, ERR_INVALID_PARAMETER);

				Vector<Vector2> array = p_arrays[ai];

				ERR_FAIL_COND_V(array.size() != p_vertex_array_len, ERR_INVALID_PARAMETER);

				const Vector2 *src = array.ptr();

				if (p_format & RS::ARRAY_FLAG_COMPRESS_ATTRIBUTES) {
					for (int i = 0; i < p_vertex_array_len; i++) {
						Vector2 vec = src[i];
						if (!r_uv_scale.is_zero_approx()) {
							// Normalize into 0-1 from possible range -uv_scale - uv_scale.
							vec = vec / (Vector2(r_uv_scale.z, r_uv_scale.w)) + Vector2(0.5, 0.5);
						}
						uint16_t uv[2] = { (uint16_t)CLAMP(vec.x * 65535, 0, 65535), (uint16_t)CLAMP(vec.y * 65535, 0, 65535) };
						memcpy(&aw[p_offsets[ai] + i * p_attrib_stride], uv, 4);
					}
				} else {
					for (int i = 0; i < p_vertex_array_len; i++) {
						float uv[2] = { (float)src[i].x, (float)src[i].y };
						memcpy(&aw[p_offsets[ai] + i * p_attrib_stride], uv, 2 * 4);
					}
				}
			} break;
			case RS::ARRAY_CUSTOM0:
			case RS::ARRAY_CUSTOM1:
			case RS::ARRAY_CUSTOM2:
			case RS::ARRAY_CUSTOM3: {
				uint32_t type = (p_format >> (ARRAY_FORMAT_CUSTOM_BASE + ARRAY_FORMAT_CUSTOM_BITS * (ai - RS::ARRAY_CUSTOM0))) & ARRAY_FORMAT_CUSTOM_MASK;
				switch (type) {
					case ARRAY_CUSTOM_RGBA8_UNORM:
					case ARRAY_CUSTOM_RGBA8_SNORM:
					case ARRAY_CUSTOM_RG_HALF: {
						// Size 4
						ERR_FAIL_COND_V(p_arrays[ai].get_type() != Variant::PACKED_BYTE_ARRAY, ERR_INVALID_PARAMETER);

						Vector<uint8_t> array = p_arrays[ai];

						ERR_FAIL_COND_V(array.size() != p_vertex_array_len * 4, ERR_INVALID_PARAMETER);

						const uint8_t *src = array.ptr();

						for (int i = 0; i < p_vertex_array_len; i++) {
							memcpy(&aw[p_offsets[ai] + i * p_attrib_stride], &src[i * 4], 4);
						}

					} break;
					case ARRAY_CUSTOM_RGBA_HALF: {
						// Size 8
						ERR_FAIL_COND_V(p_arrays[ai].get_type() != Variant::PACKED_BYTE_ARRAY, ERR_INVALID_PARAMETER);

						Vector<uint8_t> array = p_arrays[ai];

						ERR_FAIL_COND_V(array.size() != p_vertex_array_len * 8, ERR_INVALID_PARAMETER);

						const uint8_t *src = array.ptr();

						for (int i = 0; i < p_vertex_array_len; i++) {
							memcpy(&aw[p_offsets[ai] + i * p_attrib_stride], &src[i * 8], 8);
						}
					} break;
					case ARRAY_CUSTOM_R_FLOAT:
					case ARRAY_CUSTOM_RG_FLOAT:
					case ARRAY_CUSTOM_RGB_FLOAT:
					case ARRAY_CUSTOM_RGBA_FLOAT: {
						// RF
						ERR_FAIL_COND_V(p_arrays[ai].get_type() != Variant::PACKED_FLOAT32_ARRAY, ERR_INVALID_PARAMETER);

						Vector<float> array = p_arrays[ai];
						int32_t s = type - ARRAY_CUSTOM_R_FLOAT + 1;

						ERR_FAIL_COND_V(array.size() != p_vertex_array_len * s, ERR_INVALID_PARAMETER);

						const float *src = array.ptr();

						for (int i = 0; i < p_vertex_array_len; i++) {
							memcpy(&aw[p_offsets[ai] + i * p_attrib_stride], &src[i * s], sizeof(float) * s);
						}
					} break;
					default: {
					}
				}

			} break;
			case RS::ARRAY_WEIGHTS: {
				Variant::Type type = p_arrays[ai].get_type();
				ERR_FAIL_COND_V(type != Variant::PACKED_FLOAT32_ARRAY && type != Variant::PACKED_FLOAT64_ARRAY, ERR_INVALID_PARAMETER);
				uint32_t bone_count = (p_format & ARRAY_FLAG_USE_8_BONE_WEIGHTS) ? 8 : 4;
				if (type == Variant::PACKED_FLOAT32_ARRAY) {
					Vector<float> array = p_arrays[ai];
					ERR_FAIL_COND_V(array.size() != (int32_t)(p_vertex_array_len * bone_count), ERR_INVALID_PARAMETER);
					const float *src = array.ptr();
					{
						uint16_t data[8];
						for (int i = 0; i < p_vertex_array_len; i++) {
							for (uint32_t j = 0; j < bone_count; j++) {
								data[j] = CLAMP(src[i * bone_count + j] * 65535, 0, 65535);
							}

							memcpy(&sw[p_offsets[ai] + i * p_skin_stride], data, 2 * bone_count);
						}
					}
				} else { // PACKED_FLOAT64_ARRAY
					Vector<double> array = p_arrays[ai];
					ERR_FAIL_COND_V(array.size() != (int32_t)(p_vertex_array_len * bone_count), ERR_INVALID_PARAMETER);
					const double *src = array.ptr();
					{
						uint16_t data[8];
						for (int i = 0; i < p_vertex_array_len; i++) {
							for (uint32_t j = 0; j < bone_count; j++) {
								data[j] = CLAMP(src[i * bone_count + j] * 65535, 0, 65535);
							}

							memcpy(&sw[p_offsets[ai] + i * p_skin_stride], data, 2 * bone_count);
						}
					}
				}
			} break;
			case RS::ARRAY_BONES: {
				ERR_FAIL_COND_V(p_arrays[ai].get_type() != Variant::PACKED_INT32_ARRAY && p_arrays[ai].get_type() != Variant::PACKED_FLOAT32_ARRAY, ERR_INVALID_PARAMETER);

				Vector<int> array = p_arrays[ai];

				uint32_t bone_count = (p_format & ARRAY_FLAG_USE_8_BONE_WEIGHTS) ? 8 : 4;

				ERR_FAIL_COND_V(array.size() != (int32_t)(p_vertex_array_len * bone_count), ERR_INVALID_PARAMETER);

				const int *src = array.ptr();

				uint16_t data[8];

				for (int i = 0; i < p_vertex_array_len; i++) {
					for (uint32_t j = 0; j < bone_count; j++) {
						data[j] = src[i * bone_count + j];
						max_bone = MAX(data[j], max_bone);
					}

					memcpy(&sw[p_offsets[ai] + i * p_skin_stride], data, 2 * bone_count);
				}

			} break;

			case RS::ARRAY_INDEX: {
				ERR_FAIL_NULL_V(iw, ERR_INVALID_DATA);
				ERR_FAIL_COND_V(p_index_array_len <= 0, ERR_INVALID_DATA);
				ERR_FAIL_COND_V(p_arrays[ai].get_type() != Variant::PACKED_INT32_ARRAY, ERR_INVALID_PARAMETER);

				Vector<int> indices = p_arrays[ai];
				ERR_FAIL_COND_V(indices.is_empty(), ERR_INVALID_PARAMETER);
				ERR_FAIL_COND_V(indices.size() != p_index_array_len, ERR_INVALID_PARAMETER);

				/* determine whether using 16 or 32 bits indices */

				const int *src = indices.ptr();

				for (int i = 0; i < p_index_array_len; i++) {
					if (p_vertex_array_len <= (1 << 16) && p_vertex_array_len > 0) {
						uint16_t v = src[i];

						memcpy(&iw[i * 2], &v, 2);
					} else {
						uint32_t v = src[i];

						memcpy(&iw[i * 4], &v, 4);
					}
				}
			} break;
			default: {
				ERR_FAIL_V(ERR_INVALID_DATA);
			}
		}
	}

	if (p_format & RS::ARRAY_FORMAT_BONES) {
		// Create AABBs for each detected bone.
		int total_bones = max_bone + 1;

		bool first = r_bone_aabb.size() == 0;

		r_bone_aabb.resize(total_bones);

		int weight_count = (p_format & ARRAY_FLAG_USE_8_BONE_WEIGHTS) ? 8 : 4;

		if (first) {
			for (int i = 0; i < total_bones; i++) {
				r_bone_aabb.write[i].size = Vector3(-1, -1, -1); // Negative means unused.
			}
		}

		Vector<Vector3> vertices = p_arrays[RS::ARRAY_VERTEX];
		Vector<int> bones = p_arrays[RS::ARRAY_BONES];
		Vector<float> weights = p_arrays[RS::ARRAY_WEIGHTS];

		bool any_valid = false;

		if (vertices.size() && bones.size() == vertices.size() * weight_count && weights.size() == bones.size()) {
			int vs = vertices.size();
			const Vector3 *rv = vertices.ptr();
			const int *rb = bones.ptr();
			const float *rw = weights.ptr();

			AABB *bptr = r_bone_aabb.ptrw();

			for (int i = 0; i < vs; i++) {
				Vector3 v = rv[i];
				for (int j = 0; j < weight_count; j++) {
					int idx = rb[i * weight_count + j];
					float w = rw[i * weight_count + j];
					if (w == 0) {
						continue; //break;
					}
					ERR_FAIL_INDEX_V(idx, total_bones, ERR_INVALID_DATA);

					if (bptr[idx].size.x < 0) {
						// First
						bptr[idx] = AABB(v, SMALL_VEC3);
						any_valid = true;
					} else {
						bptr[idx].expand_to(v);
					}
				}
			}
		}

		if (!any_valid && first) {
			r_bone_aabb.clear();
		}
	}
	return OK;
}

Array lain::RenderingSystem::mesh_surface_get_arrays(RID p_mesh, int p_surface) const {
	SurfaceData sd = mesh_get_surface(p_mesh, p_surface);
	return mesh_create_arrays_from_surface_data(sd);
}

Array RenderingSystem::mesh_create_arrays_from_surface_data(const SurfaceData &p_data) const {
	Vector<uint8_t> vertex_data = p_data.vertex_data;
	Vector<uint8_t> attrib_data = p_data.attribute_data;
	Vector<uint8_t> skin_data = p_data.skin_data;

	ERR_FAIL_COND_V(vertex_data.is_empty() && (p_data.format & RS::ARRAY_FORMAT_VERTEX), Array());
	int vertex_len = p_data.vertex_count;

	Vector<uint8_t> index_data = p_data.index_data;
	int index_len = p_data.index_count;

	uint64_t format = p_data.format;

	return _get_array_from_surface(format, vertex_data, attrib_data, skin_data, vertex_len, index_data, index_len, p_data.aabb, p_data.uv_scale);
}
// 就这步，获得LOD的方法是
Dictionary RS::mesh_surface_get_lods(RID p_mesh, int p_surface) const {
	SurfaceData sd = mesh_get_surface(p_mesh, p_surface);
	ERR_FAIL_COND_V(sd.vertex_count == 0, Dictionary());

	Dictionary ret;

	for (int i = 0; i < sd.lods.size(); i++) {
		Vector<int> lods;
		if (sd.vertex_count <= 65536) {
			uint32_t lc = sd.lods[i].index_data.size() / 2;
			lods.resize(lc);
			const uint8_t *r = sd.lods[i].index_data.ptr();
			const uint16_t *rptr = (const uint16_t *)r;
			int *w = lods.ptrw();
			for (uint32_t j = 0; j < lc; j++) {
				w[j] = rptr[i];
			}
		} else {
			uint32_t lc = sd.lods[i].index_data.size() / 4;
			lods.resize(lc);
			const uint8_t *r = sd.lods[i].index_data.ptr();
			const uint32_t *rptr = (const uint32_t *)r;
			int *w = lods.ptrw();
			for (uint32_t j = 0; j < lc; j++) {
				w[j] = rptr[i];
			}
		}

		ret[sd.lods[i].edge_length] = lods;
	}

	return ret;
}

// The inputs to this function should match the outputs of _get_axis_angle. I.e. p_axis is a normalized vector
// and p_angle includes the binormal direction.
void _get_tbn_from_axis_angle(const Vector3 &p_axis, float p_angle, Vector3 &r_normal, Vector4 &r_tangent) {
	float binormal_sign = p_angle > 0.5 ? 1.0 : -1.0;
	float angle = Math::abs(p_angle * 2.0 - 1.0) * Math_PI;

	Basis tbn = Basis(p_axis, angle);
	Vector3 tan = tbn.rows[0];
	r_tangent = Vector4(tan.x, tan.y, tan.z, binormal_sign);
	r_normal = tbn.rows[2];
}

Array RS::_get_array_from_surface(uint64_t p_format, Vector<uint8_t> p_vertex_data, Vector<uint8_t> p_attrib_data, Vector<uint8_t> p_skin_data, int p_vertex_len, Vector<uint8_t> p_index_data, int p_index_len, const AABB &p_aabb, const Vector4 &p_uv_scale) const {
	uint32_t offsets[RS::ARRAY_MAX];

	uint32_t vertex_elem_size;
	uint32_t normal_elem_size;
	uint32_t attrib_elem_size;
	uint32_t skin_elem_size;
	mesh_surface_make_offsets_from_format(p_format, p_vertex_len, p_index_len, offsets, vertex_elem_size, normal_elem_size, attrib_elem_size, skin_elem_size);

	Array ret;
	ret.resize(RS::ARRAY_MAX);

	const uint8_t *r = p_vertex_data.ptr();
	const uint8_t *ar = p_attrib_data.ptr();
	const uint8_t *sr = p_skin_data.ptr();

	for (int i = 0; i < RS::ARRAY_MAX; i++) {
		if (!(p_format & (1ULL << i))) {
			continue;
		}

		switch (i) {
			case RS::ARRAY_VERTEX: {
				if (p_format & ARRAY_FLAG_USE_2D_VERTICES) {
					Vector<Vector2> arr_2d;
					arr_2d.resize(p_vertex_len);

					{
						Vector2 *w = arr_2d.ptrw();

						for (int j = 0; j < p_vertex_len; j++) {
							const float *v = reinterpret_cast<const float *>(&r[j * vertex_elem_size + offsets[i]]);
							w[j] = Vector2(v[0], v[1]);
						}
					}

					ret[i] = arr_2d;
				} else {
					Vector<Vector3> arr_3d;
					arr_3d.resize(p_vertex_len);

					{
						Vector3 *w = arr_3d.ptrw();

						if (p_format & ARRAY_FLAG_COMPRESS_ATTRIBUTES) {
							// We only have vertices to read, so just read them and skip everything else.
							if (!(p_format & RS::ARRAY_FORMAT_NORMAL)) {
								for (int j = 0; j < p_vertex_len; j++) {
									const uint16_t *v = reinterpret_cast<const uint16_t *>(&r[j * vertex_elem_size + offsets[i]]);
									Vector3 vec = Vector3(float(v[0]) / 65535.0, float(v[1]) / 65535.0, float(v[2]) / 65535.0);
									w[j] = (vec * p_aabb.size) + p_aabb.position;
								}
								continue;
							}

							Vector<Vector3> normals;
							normals.resize(p_vertex_len);
							Vector3 *normalsw = normals.ptrw();

							Vector<float> tangents;
							tangents.resize(p_vertex_len * 4);
							float *tangentsw = tangents.ptrw();

							for (int j = 0; j < p_vertex_len; j++) {
								const uint32_t n = *(const uint32_t *)&r[j * normal_elem_size + offsets[RS::ARRAY_NORMAL]];
								Vector3 axis = Vector3::octahedron_decode(Vector2((n & 0xFFFF) / 65535.0, ((n >> 16) & 0xFFFF) / 65535.0));

								const uint16_t *v = reinterpret_cast<const uint16_t *>(&r[j * vertex_elem_size + offsets[i]]);
								Vector3 vec = Vector3(float(v[0]) / 65535.0, float(v[1]) / 65535.0, float(v[2]) / 65535.0);
								float angle = float(v[3]) / 65535.0;
								w[j] = (vec * p_aabb.size) + p_aabb.position;

								Vector3 normal;
								Vector4 tan;
								_get_tbn_from_axis_angle(axis, angle, normal, tan);

								normalsw[j] = normal;
								tangentsw[j * 4 + 0] = tan.x;
								tangentsw[j * 4 + 1] = tan.y;
								tangentsw[j * 4 + 2] = tan.z;
								tangentsw[j * 4 + 3] = tan.w;
							}
							ret[RS::ARRAY_NORMAL] = normals;
							ret[RS::ARRAY_TANGENT] = tangents;

						} else {
							for (int j = 0; j < p_vertex_len; j++) {
								const float *v = reinterpret_cast<const float *>(&r[j * vertex_elem_size + offsets[i]]);
								w[j] = Vector3(v[0], v[1], v[2]);
							}
						}
					}

					ret[i] = arr_3d;
				}

			} break;
			case RS::ARRAY_NORMAL: {
				if (!(p_format & RS::ARRAY_FLAG_COMPRESS_ATTRIBUTES)) {
					Vector<Vector3> arr;
					arr.resize(p_vertex_len);

					Vector3 *w = arr.ptrw();

					for (int j = 0; j < p_vertex_len; j++) {
						const uint32_t v = *(const uint32_t *)&r[j * normal_elem_size + offsets[i]];

						w[j] = Vector3::octahedron_decode(Vector2((v & 0xFFFF) / 65535.0, ((v >> 16) & 0xFFFF) / 65535.0));
					}

					ret[i] = arr;
				}
			} break;

			case RS::ARRAY_TANGENT: {
				if (!(p_format & RS::ARRAY_FLAG_COMPRESS_ATTRIBUTES)) {
					Vector<float> arr;
					arr.resize(p_vertex_len * 4);

					float *w = arr.ptrw();

					for (int j = 0; j < p_vertex_len; j++) {
						const uint32_t v = *(const uint32_t *)&r[j * normal_elem_size + offsets[i]];
						float tangent_sign;
						Vector3 res = Vector3::octahedron_tangent_decode(Vector2((v & 0xFFFF) / 65535.0, ((v >> 16) & 0xFFFF) / 65535.0), &tangent_sign);
						w[j * 4 + 0] = res.x;
						w[j * 4 + 1] = res.y;
						w[j * 4 + 2] = res.z;
						w[j * 4 + 3] = tangent_sign;
					}

					ret[i] = arr;
				}
			} break;
			case RS::ARRAY_COLOR: {
				Vector<Color> arr;
				arr.resize(p_vertex_len);

				Color *w = arr.ptrw();

				for (int32_t j = 0; j < p_vertex_len; j++) {
					const uint8_t *v = reinterpret_cast<const uint8_t *>(&ar[j * attrib_elem_size + offsets[i]]);

					w[j] = Color(v[0] / 255.0, v[1] / 255.0, v[2] / 255.0, v[3] / 255.0);
				}

				ret[i] = arr;
			} break;
			case RS::ARRAY_TEX_UV: {
				Vector<Vector2> arr;
				arr.resize(p_vertex_len);

				Vector2 *w = arr.ptrw();
				if (p_format & ARRAY_FLAG_COMPRESS_ATTRIBUTES) {
					for (int j = 0; j < p_vertex_len; j++) {
						const uint16_t *v = reinterpret_cast<const uint16_t *>(&ar[j * attrib_elem_size + offsets[i]]);
						Vector2 vec = Vector2(float(v[0]) / 65535.0, float(v[1]) / 65535.0);
						if (!p_uv_scale.is_zero_approx()) {
							vec = (vec - Vector2(0.5, 0.5)) * Vector2(p_uv_scale.x, p_uv_scale.y);
						}

						w[j] = vec;
					}
				} else {
					for (int j = 0; j < p_vertex_len; j++) {
						const float *v = reinterpret_cast<const float *>(&ar[j * attrib_elem_size + offsets[i]]);
						w[j] = Vector2(v[0], v[1]);
					}
				}
				ret[i] = arr;
			} break;

			case RS::ARRAY_TEX_UV2: {
				Vector<Vector2> arr;
				arr.resize(p_vertex_len);

				Vector2 *w = arr.ptrw();

				if (p_format & ARRAY_FLAG_COMPRESS_ATTRIBUTES) {
					for (int j = 0; j < p_vertex_len; j++) {
						const uint16_t *v = reinterpret_cast<const uint16_t *>(&ar[j * attrib_elem_size + offsets[i]]);
						Vector2 vec = Vector2(float(v[0]) / 65535.0, float(v[1]) / 65535.0);
						if (!p_uv_scale.is_zero_approx()) {
							vec = (vec - Vector2(0.5, 0.5)) * Vector2(p_uv_scale.z, p_uv_scale.w);
						}
						w[j] = vec;
					}
				} else {
					for (int j = 0; j < p_vertex_len; j++) {
						const float *v = reinterpret_cast<const float *>(&ar[j * attrib_elem_size + offsets[i]]);
						w[j] = Vector2(v[0], v[1]);
					}
				}

				ret[i] = arr;

			} break;
			case RS::ARRAY_CUSTOM0:
			case RS::ARRAY_CUSTOM1:
			case RS::ARRAY_CUSTOM2:
			case RS::ARRAY_CUSTOM3: {
				uint32_t type = (p_format >> (ARRAY_FORMAT_CUSTOM_BASE + ARRAY_FORMAT_CUSTOM_BITS * (i - RS::ARRAY_CUSTOM0))) & ARRAY_FORMAT_CUSTOM_MASK;
				switch (type) {
					case ARRAY_CUSTOM_RGBA8_UNORM:
					case ARRAY_CUSTOM_RGBA8_SNORM:
					case ARRAY_CUSTOM_RG_HALF:
					case ARRAY_CUSTOM_RGBA_HALF: {
						// Size 4
						int s = type == ARRAY_CUSTOM_RGBA_HALF ? 8 : 4;
						Vector<uint8_t> arr;
						arr.resize(p_vertex_len * s);

						uint8_t *w = arr.ptrw();

						for (int j = 0; j < p_vertex_len; j++) {
							const uint8_t *v = reinterpret_cast<const uint8_t *>(&ar[j * attrib_elem_size + offsets[i]]);
							memcpy(&w[j * s], v, s);
						}

						ret[i] = arr;

					} break;
					case ARRAY_CUSTOM_R_FLOAT:
					case ARRAY_CUSTOM_RG_FLOAT:
					case ARRAY_CUSTOM_RGB_FLOAT:
					case ARRAY_CUSTOM_RGBA_FLOAT: {
						uint32_t s = type - ARRAY_CUSTOM_R_FLOAT + 1;

						Vector<float> arr;
						arr.resize(s * p_vertex_len);

						float *w = arr.ptrw();

						for (int j = 0; j < p_vertex_len; j++) {
							const float *v = reinterpret_cast<const float *>(&ar[j * attrib_elem_size + offsets[i]]);
							memcpy(&w[j * s], v, s * sizeof(float));
						}
						ret[i] = arr;

					} break;
					default: {
					}
				}

			} break;
			case RS::ARRAY_WEIGHTS: {
				uint32_t bone_count = (p_format & ARRAY_FLAG_USE_8_BONE_WEIGHTS) ? 8 : 4;

				Vector<float> arr;
				arr.resize(p_vertex_len * bone_count);
				{
					float *w = arr.ptrw();

					for (int j = 0; j < p_vertex_len; j++) {
						const uint16_t *v = (const uint16_t *)&sr[j * skin_elem_size + offsets[i]];
						for (uint32_t k = 0; k < bone_count; k++) {
							w[j * bone_count + k] = float(v[k] / 65535.0);
						}
					}
				}

				ret[i] = arr;

			} break;
			case RS::ARRAY_BONES: {
				uint32_t bone_count = (p_format & ARRAY_FLAG_USE_8_BONE_WEIGHTS) ? 8 : 4;

				Vector<int> arr;
				arr.resize(p_vertex_len * bone_count);

				int *w = arr.ptrw();

				for (int j = 0; j < p_vertex_len; j++) {
					const uint16_t *v = (const uint16_t *)&sr[j * skin_elem_size + offsets[i]];
					for (uint32_t k = 0; k < bone_count; k++) {
						w[j * bone_count + k] = v[k];
					}
				}

				ret[i] = arr;

			} break;
			case RS::ARRAY_INDEX: {
				/* determine whether using 16 or 32 bits indices */

				const uint8_t *ir = p_index_data.ptr();

				Vector<int> arr;
				arr.resize(p_index_len);
				if (p_vertex_len <= (1 << 16)) {
					int *w = arr.ptrw();

					for (int j = 0; j < p_index_len; j++) {
						const uint16_t *v = (const uint16_t *)&ir[j * 2];
						w[j] = *v;
					}
				} else {
					int *w = arr.ptrw();

					for (int j = 0; j < p_index_len; j++) {
						const int *v = (const int *)&ir[j * 4];
						w[j] = *v;
					}
				}
				ret[i] = arr;
			} break;
			default: {
				ERR_FAIL_V(ret);
			}
		}
	}

	return ret;
}
