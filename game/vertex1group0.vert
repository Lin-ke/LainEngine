#version 450


#define MAX_ROUGHNESS_LOD 7.0

#define USE_RADIANCE_CUBEMAP_ARRAY 

#define SDFGI_OCT_SIZE 1

#define MAX_DIRECTIONAL_LIGHT_DATA_STRUCTS 8

#define MAX_LIGHTMAP_TEXTURES 8

#define MAX_LIGHTMAPS 8

#define MATERIAL_UNIFORM_SET 3

#define MODE_RENDER_DEPTH
#define MODE_DUAL_PARABOLOID
#define DEBUG_DRAW_PSSM_SPLITS
#define FOG_DISABLED

#define FRAGMENT_CODE_USED
#define RENDER_DRIVER_VULKAN
#define M_PI 3.14159265359
#define ROUGHNESS_MAX_LOD 5
#define MAX_VOXEL_GI_INSTANCES 8
#define MAX_VIEWS 2
#ifndef MOLTENVK_USED
#if defined(has_GL_KHR_shader_subgroup_ballot) && defined(has_GL_KHR_shader_subgroup_arithmetic)
#extension GL_KHR_shader_subgroup_ballot : enable
#extension GL_KHR_shader_subgroup_arithmetic : enable
#define USE_SUBGROUPS
#endif
#endif 
#if defined(USE_MULTIVIEW) && defined(has_VK_KHR_multiview)
#extension GL_EXT_multiview : enable
#endif
#define CLUSTER_COUNTER_SHIFT 20
#define CLUSTER_POINTER_MASK ((1 << CLUSTER_COUNTER_SHIFT) - 1)
#define CLUSTER_COUNTER_MASK 0xfff
struct DecalData {
	highp mat4 xform; 
	highp vec3 inv_extents;
	mediump float albedo_mix;
	highp vec4 albedo_rect;
	highp vec4 normal_rect;
	highp vec4 orm_rect;
	highp vec4 emission_rect;
	highp vec4 modulate;
	mediump float emission_energy;
	uint mask;
	mediump float upper_fade;
	mediump float lower_fade;
	mediump mat3x4 normal_xform;
	mediump vec3 normal;
	mediump float normal_fade;
};
struct SceneData {
	highp mat4 projection_matrix;
	highp mat4 inv_projection_matrix;
	highp mat4 inv_view_matrix;
	highp mat4 view_matrix;
	
	highp mat4 projection_matrix_view[MAX_VIEWS];
	highp mat4 inv_projection_matrix_view[MAX_VIEWS];
	highp vec4 eye_offset[MAX_VIEWS];
	
	highp mat4 main_cam_inv_view_matrix;
	highp vec2 viewport_size;
	highp vec2 screen_pixel_size;
	
	highp vec4 directional_penumbra_shadow_kernel[32];
	highp vec4 directional_soft_shadow_kernel[32];
	highp vec4 penumbra_shadow_kernel[32];
	highp vec4 soft_shadow_kernel[32];
	mediump mat3 radiance_inverse_xform;
	mediump vec4 ambient_light_color_energy;
	mediump float ambient_color_sky_mix;
	bool use_ambient_light;
	bool use_ambient_cubemap;
	bool use_reflection_cubemap;
	highp vec2 shadow_atlas_pixel_size;
	highp vec2 directional_shadow_pixel_size;
	uint directional_light_count;
	mediump float dual_paraboloid_side;
	highp float z_far;
	highp float z_near;
	bool roughness_limiter_enabled;
	mediump float roughness_limiter_amount;
	mediump float roughness_limiter_limit;
	mediump float opaque_prepass_threshold;
	bool fog_enabled;
	uint fog_mode;
	highp float fog_density;
	highp float fog_height;
	highp float fog_height_density;
	highp float fog_depth_curve;
	highp float pad;
	highp float fog_depth_begin;
	mediump vec3 fog_light_color;
	highp float fog_depth_end;
	mediump float fog_sun_scatter;
	mediump float fog_aerial_perspective;
	highp float time;
	mediump float reflection_multiplier; 
	vec2 taa_jitter;
	bool material_uv2_mode;
	float emissive_exposure_normalization;
	float IBL_exposure_normalization;
	bool pancake_shadows;
	uint camera_visible_layers;
	float pass_alpha_multiplier;
};
#if !defined(MODE_RENDER_DEPTH) || defined(MODE_RENDER_MATERIAL) || defined(MODE_RENDER_SDF) || defined(MODE_RENDER_NORMAL_ROUGHNESS) || defined(MODE_RENDER_VOXEL_GI) || defined(TANGENT_USED) || defined(NORMAL_MAP_USED) || defined(LIGHT_ANISOTROPY_USED)
#ifndef NORMAL_USED
#define NORMAL_USED
#endif
#endif
#if !defined(TANGENT_USED) && (defined(NORMAL_MAP_USED) || defined(LIGHT_ANISOTROPY_USED))
#define TANGENT_USED
#endif
layout(push_constant, std430) uniform DrawCall {
	uint instance_index;
	uint uv_offset;
	uint multimesh_motion_vectors_current_offset;
	uint multimesh_motion_vectors_previous_offset;
}
draw_call;
#define SDFGI_MAX_CASCADES 8
/* Set 0: Base Pass (never changes) */
#define LIGHT_BAKE_DISABLED 0
#define LIGHT_BAKE_STATIC 1
#define LIGHT_BAKE_DYNAMIC 2
struct LightData { 
	highp vec3 position;
	highp float inv_radius;
	mediump vec3 direction;
	highp float size;
	mediump vec3 color;
	mediump float attenuation;
	mediump float cone_attenuation;
	mediump float cone_angle;
	mediump float specular_amount;
	mediump float shadow_opacity;
	highp vec4 atlas_rect; 
	highp mat4 shadow_matrix;
	highp float shadow_bias;
	highp float shadow_normal_bias;
	highp float transmittance_bias;
	highp float soft_shadow_size; 
	highp float soft_shadow_scale; 
	uint mask;
	mediump float volumetric_fog_energy;
	uint bake_mode;
	highp vec4 projector_rect; 
};
#define REFLECTION_AMBIENT_DISABLED 0
#define REFLECTION_AMBIENT_ENVIRONMENT 1
#define REFLECTION_AMBIENT_COLOR 2
struct ReflectionData {
	highp vec3 box_extents;
	mediump float index;
	highp vec3 box_offset;
	uint mask;
	mediump vec3 ambient; 
	mediump float intensity;
	bool exterior;
	bool box_project;
	uint ambient_mode;
	float exposure_normalization;
	
	highp mat4 local_matrix; 
	
};
struct DirectionalLightData {
	mediump vec3 direction;
	highp float energy; 
	mediump vec3 color;
	mediump float size;
	mediump float specular;
	uint mask;
	highp float softshadow_angle;
	highp float soft_shadow_scale;
	bool blend_splits;
	mediump float shadow_opacity;
	highp float fade_from;
	highp float fade_to;
	uvec2 pad;
	uint bake_mode;
	mediump float volumetric_fog_energy;
	highp vec4 shadow_bias;
	highp vec4 shadow_normal_bias;
	highp vec4 shadow_transmittance_bias;
	highp vec4 shadow_z_range;
	highp vec4 shadow_range_begin;
	highp vec4 shadow_split_offsets;
	highp mat4 shadow_matrix1;
	highp mat4 shadow_matrix2;
	highp mat4 shadow_matrix3;
	highp mat4 shadow_matrix4;
	highp vec2 uv_scale1;
	highp vec2 uv_scale2;
	highp vec2 uv_scale3;
	highp vec2 uv_scale4;
};
layout(set = 0, binding = 2) uniform sampler shadow_sampler;
#define INSTANCE_FLAGS_DYNAMIC (1 << 3)
#define INSTANCE_FLAGS_NON_UNIFORM_SCALE (1 << 4)
#define INSTANCE_FLAGS_USE_GI_BUFFERS (1 << 5)
#define INSTANCE_FLAGS_USE_SDFGI (1 << 6)
#define INSTANCE_FLAGS_USE_LIGHTMAP_CAPTURE (1 << 7)
#define INSTANCE_FLAGS_USE_LIGHTMAP (1 << 8)
#define INSTANCE_FLAGS_USE_SH_LIGHTMAP (1 << 9)
#define INSTANCE_FLAGS_USE_VOXEL_GI (1 << 10)
#define INSTANCE_FLAGS_PARTICLES (1 << 11)
#define INSTANCE_FLAGS_MULTIMESH (1 << 12)
#define INSTANCE_FLAGS_MULTIMESH_FORMAT_2D (1 << 13)
#define INSTANCE_FLAGS_MULTIMESH_HAS_COLOR (1 << 14)
#define INSTANCE_FLAGS_MULTIMESH_HAS_CUSTOM_DATA (1 << 15)
#define INSTANCE_FLAGS_PARTICLE_TRAIL_SHIFT 16
#define INSTANCE_FLAGS_FADE_SHIFT 24
#define INSTANCE_FLAGS_PARTICLE_TRAIL_MASK 0xFF
#define SCREEN_SPACE_EFFECTS_FLAGS_USE_SSAO 1
#define SCREEN_SPACE_EFFECTS_FLAGS_USE_SSIL 2
layout(set = 0, binding = 3, std430) restrict readonly buffer OmniLights {
	LightData data[];
}
omni_lights;
layout(set = 0, binding = 4, std430) restrict readonly buffer SpotLights {
	LightData data[];
}
spot_lights;
layout(set = 0, binding = 5, std430) restrict readonly buffer ReflectionProbeData {
	ReflectionData data[];
}
reflections;
layout(set = 0, binding = 6, std140) uniform DirectionalLights {
	DirectionalLightData data[MAX_DIRECTIONAL_LIGHT_DATA_STRUCTS];
}
directional_lights;
#define LIGHTMAP_FLAG_USE_DIRECTION 1
#define LIGHTMAP_FLAG_USE_SPECULAR_DIRECTION 2
struct Lightmap {
	mat3 normal_xform;
	vec3 pad;
	float exposure_normalization;
};
layout(set = 0, binding = 7, std140) restrict readonly buffer Lightmaps {
	Lightmap data[];
}
lightmaps;
struct LightmapCapture {
	vec4 sh[9];
};
layout(set = 0, binding = 8, std140) restrict readonly buffer LightmapCaptures {
	LightmapCapture data[];
}
lightmap_captures;
layout(set = 0, binding = 9) uniform texture2D decal_atlas;
layout(set = 0, binding = 10) uniform texture2D decal_atlas_srgb;
layout(set = 0, binding = 11, std430) restrict readonly buffer Decals {
	DecalData data[];
}
decals;
layout(set = 0, binding = 12, std430) restrict readonly buffer GlobalShaderUniformData {
	vec4 data[];
}
global_shader_uniforms;
struct SDFVoxelGICascadeData {
	vec3 position;
	float to_probe;
	ivec3 probe_world_offset;
	float to_cell; 
	vec3 pad;
	float exposure_normalization;
};
layout(set = 0, binding = 13, std140) uniform SDFGI {
	vec3 grid_size;
	uint max_cascades;
	bool use_occlusion;
	int probe_axis_size;
	float probe_to_uvw;
	float normal_bias;
	vec3 lightprobe_tex_pixel_size;
	float energy;
	vec3 lightprobe_uv_offset;
	float y_mult;
	vec3 occlusion_clamp;
	uint pad3;
	vec3 occlusion_renormalize;
	uint pad4;
	vec3 cascade_probe_size;
	uint pad5;
	SDFVoxelGICascadeData cascades[SDFGI_MAX_CASCADES];
}
sdfgi;
layout(set = 0, binding = 14) uniform sampler DEFAULT_SAMPLER_LINEAR_WITH_MIPMAPS_CLAMP;
layout(set = 0, binding = 15) uniform texture2D best_fit_normal_texture;
/* Set 1: Render Pass (changes per render pass) */
layout(set = 1, binding = 0, std140) uniform SceneDataBlock {
	SceneData data;
	SceneData prev_data;
}
scene_data_block;
struct ImplementationData {
	uint cluster_shift;
	uint cluster_width;
	uint cluster_type_size;
	uint max_cluster_element_count_div_32;
	uint ss_effects_flags;
	float ssao_light_affect;
	float ssao_ao_affect;
	uint pad1;
	mat4 sdf_to_bounds;
	ivec3 sdf_offset;
	uint pad2;
	ivec3 sdf_size;
	bool gi_upscale_for_msaa;
	bool volumetric_fog_enabled;
	float volumetric_fog_inv_length;
	float volumetric_fog_detail_spread;
	uint volumetric_fog_pad;
};
layout(set = 1, binding = 1, std140) uniform ImplementationDataBlock {
	ImplementationData data;
}
implementation_data_block;
#define implementation_data implementation_data_block.data
struct InstanceData {
	mat4 transform;
	mat4 prev_transform;
	uint flags;
	uint instance_uniforms_ofs; 
	uint gi_offset; 
	uint layer_mask;
	vec4 lightmap_uv_scale;
	vec4 compressed_aabb_position_pad; 
	vec4 compressed_aabb_size_pad; 
	vec4 uv_scale;
};
layout(set = 1, binding = 2, std430) buffer restrict readonly InstanceDataBuffer {
	InstanceData data[];
}
instances;
#ifdef USE_RADIANCE_CUBEMAP_ARRAY
layout(set = 1, binding = 3) uniform textureCubeArray radiance_cubemap;
#else
layout(set = 1, binding = 3) uniform textureCube radiance_cubemap;
#endif
layout(set = 1, binding = 4) uniform textureCubeArray reflection_atlas;
layout(set = 1, binding = 5) uniform texture2D shadow_atlas;
layout(set = 1, binding = 6) uniform texture2D directional_shadow_atlas;
layout(set = 1, binding = 7) uniform texture2DArray lightmap_textures[MAX_LIGHTMAP_TEXTURES];
layout(set = 1, binding = 8) uniform texture3D voxel_gi_textures[MAX_VOXEL_GI_INSTANCES];
layout(set = 1, binding = 9, std430) buffer restrict readonly ClusterBuffer {
	uint data[];
}
cluster_buffer;
layout(set = 1, binding = 10) uniform sampler decal_sampler;
layout(set = 1, binding = 11) uniform sampler light_projector_sampler;
layout(set = 1, binding = 12 + 0) uniform sampler SAMPLER_NEAREST_CLAMP;
layout(set = 1, binding = 12 + 1) uniform sampler SAMPLER_LINEAR_CLAMP;
layout(set = 1, binding = 12 + 2) uniform sampler SAMPLER_NEAREST_WITH_MIPMAPS_CLAMP;
layout(set = 1, binding = 12 + 3) uniform sampler SAMPLER_LINEAR_WITH_MIPMAPS_CLAMP;
layout(set = 1, binding = 12 + 4) uniform sampler SAMPLER_NEAREST_WITH_MIPMAPS_ANISOTROPIC_CLAMP;
layout(set = 1, binding = 12 + 5) uniform sampler SAMPLER_LINEAR_WITH_MIPMAPS_ANISOTROPIC_CLAMP;
layout(set = 1, binding = 12 + 6) uniform sampler SAMPLER_NEAREST_REPEAT;
layout(set = 1, binding = 12 + 7) uniform sampler SAMPLER_LINEAR_REPEAT;
layout(set = 1, binding = 12 + 8) uniform sampler SAMPLER_NEAREST_WITH_MIPMAPS_REPEAT;
layout(set = 1, binding = 12 + 9) uniform sampler SAMPLER_LINEAR_WITH_MIPMAPS_REPEAT;
layout(set = 1, binding = 12 + 10) uniform sampler SAMPLER_NEAREST_WITH_MIPMAPS_ANISOTROPIC_REPEAT;
layout(set = 1, binding = 12 + 11) uniform sampler SAMPLER_LINEAR_WITH_MIPMAPS_ANISOTROPIC_REPEAT;
#ifdef MODE_RENDER_SDF
layout(r16ui, set = 1, binding = 24) uniform restrict writeonly uimage3D albedo_volume_grid;
layout(r32ui, set = 1, binding = 25) uniform restrict writeonly uimage3D emission_grid;
layout(r32ui, set = 1, binding = 26) uniform restrict writeonly uimage3D emission_aniso_grid;
layout(r32ui, set = 1, binding = 27) uniform restrict uimage3D geom_facing_grid;
#define depth_buffer shadow_atlas
#define color_buffer shadow_atlas
#define normal_roughness_buffer shadow_atlas
#define multiviewSampler sampler2D
#else
#ifdef USE_MULTIVIEW
layout(set = 1, binding = 24) uniform texture2DArray depth_buffer;
layout(set = 1, binding = 25) uniform texture2DArray color_buffer;
layout(set = 1, binding = 26) uniform texture2DArray normal_roughness_buffer;
layout(set = 1, binding = 27) uniform texture2DArray ao_buffer;
layout(set = 1, binding = 28) uniform texture2DArray ambient_buffer;
layout(set = 1, binding = 29) uniform texture2DArray reflection_buffer;
#define multiviewSampler sampler2DArray
#else 
layout(set = 1, binding = 24) uniform texture2D depth_buffer;
layout(set = 1, binding = 25) uniform texture2D color_buffer;
layout(set = 1, binding = 26) uniform texture2D normal_roughness_buffer;
layout(set = 1, binding = 27) uniform texture2D ao_buffer;
layout(set = 1, binding = 28) uniform texture2D ambient_buffer;
layout(set = 1, binding = 29) uniform texture2D reflection_buffer;
#define multiviewSampler sampler2D
#endif
layout(set = 1, binding = 30) uniform texture2DArray sdfgi_lightprobe_texture;
layout(set = 1, binding = 31) uniform texture3D sdfgi_occlusion_cascades;
struct VoxelGIData {
	mat4 xform; 
	vec3 bounds; 
	float dynamic_range; 
	float bias; 
	float normal_bias; 
	bool blend_ambient; 
	uint mipmaps; 
	vec3 pad; 
	float exposure_normalization; 
};
layout(set = 1, binding = 32, std140) uniform VoxelGIs {
	VoxelGIData data[MAX_VOXEL_GI_INSTANCES];
}
voxel_gi_instances;
layout(set = 1, binding = 33) uniform texture3D volumetric_fog_texture;
#ifdef USE_MULTIVIEW
layout(set = 1, binding = 34) uniform texture2DArray ssil_buffer;
#else
layout(set = 1, binding = 34) uniform texture2D ssil_buffer;
#endif 
#endif
vec4 normal_roughness_compatibility(vec4 p_normal_roughness) {
	float roughness = p_normal_roughness.w;
	if (roughness > 0.5) {
		roughness = 1.0 - roughness;
	}
	roughness /= (127.0 / 255.0);
	return vec4(normalize(p_normal_roughness.xyz * 2.0 - 1.0) * 0.5 + 0.5, roughness);
}
/* Set 2 Skeleton & Instancing (can change per item) */
layout(set = 2, binding = 0, std430) restrict readonly buffer Transforms {
	vec4 data[];
}
transforms;
/* Set 3 User Material */
#define SHADER_IS_SRGB false
/* INPUT ATTRIBS */
layout(location = 0) in vec4 vertex_angle_attrib;
#if defined(NORMAL_USED) || defined(TANGENT_USED)
layout(location = 1) in vec4 axis_tangent_attrib;
#endif
#if defined(COLOR_USED)
layout(location = 3) in vec4 color_attrib;
#endif
#ifdef UV_USED
layout(location = 4) in vec2 uv_attrib;
#endif
#if defined(UV2_USED) || defined(USE_LIGHTMAP) || defined(MODE_RENDER_MATERIAL)
layout(location = 5) in vec2 uv2_attrib;
#endif
#if defined(CUSTOM0_USED)
layout(location = 6) in vec4 custom0_attrib;
#endif
#if defined(CUSTOM1_USED)
layout(location = 7) in vec4 custom1_attrib;
#endif
#if defined(CUSTOM2_USED)
layout(location = 8) in vec4 custom2_attrib;
#endif
#if defined(CUSTOM3_USED)
layout(location = 9) in vec4 custom3_attrib;
#endif
#if defined(BONES_USED) || defined(USE_PARTICLE_TRAILS)
layout(location = 10) in uvec4 bone_attrib;
#endif
#if defined(WEIGHTS_USED) || defined(USE_PARTICLE_TRAILS)
layout(location = 11) in vec4 weight_attrib;
#endif
#ifdef MOTION_VECTORS
layout(location = 12) in vec4 previous_vertex_attrib;
#if defined(NORMAL_USED) || defined(TANGENT_USED)
layout(location = 13) in vec4 previous_normal_attrib;
#endif
#endif 
vec3 oct_to_vec3(vec2 e) {
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	float t = max(-v.z, 0.0);
	v.xy += t * -sign(v.xy);
	return normalize(v);
}
void axis_angle_to_tbn(vec3 axis, float angle, out vec3 tangent, out vec3 binormal, out vec3 normal) {
	float c = cos(angle);
	float s = sin(angle);
	vec3 omc_axis = (1.0 - c) * axis;
	vec3 s_axis = s * axis;
	tangent = omc_axis.xxx * axis + vec3(c, -s_axis.z, s_axis.y);
	binormal = omc_axis.yyy * axis + vec3(s_axis.z, c, -s_axis.x);
	normal = omc_axis.zzz * axis + vec3(-s_axis.y, s_axis.x, c);
}
/* Varyings */
layout(location = 0) out vec3 vertex_interp;
#ifdef NORMAL_USED
layout(location = 1) out vec3 normal_interp;
#endif
#if defined(COLOR_USED)
layout(location = 2) out vec4 color_interp;
#endif
#ifdef UV_USED
layout(location = 3) out vec2 uv_interp;
#endif
#if defined(UV2_USED) || defined(USE_LIGHTMAP)
layout(location = 4) out vec2 uv2_interp;
#endif
#ifdef TANGENT_USED
layout(location = 5) out vec3 tangent_interp;
layout(location = 6) out vec3 binormal_interp;
#endif
#ifdef MOTION_VECTORS
layout(location = 7) out vec4 screen_position;
layout(location = 8) out vec4 prev_screen_position;
#endif
#ifdef MATERIAL_UNIFORMS_USED
layout(set = MATERIAL_UNIFORM_SET, binding = 0, std140) uniform MaterialUniforms{
} material;
#endif
float global_time;
#ifdef MODE_DUAL_PARABOLOID
layout(location = 9) out float dp_clip;
#endif
layout(location = 10) out flat uint instance_index_interp;
#ifdef USE_MULTIVIEW
#ifdef has_VK_KHR_multiview
#define ViewIndex gl_ViewIndex
#else 
#define ViewIndex 0
#endif 
vec3 multiview_uv(vec2 uv) {
	return vec3(uv, ViewIndex);
}
ivec3 multiview_uv(ivec2 uv) {
	return ivec3(uv, int(ViewIndex));
}
layout(location = 11) out vec4 combined_projected;
#else 
#define ViewIndex 0
vec2 multiview_uv(vec2 uv) {
	return uv;
}
ivec2 multiview_uv(ivec2 uv) {
	return uv;
}
#endif 
invariant gl_Position;
#ifdef USE_DOUBLE_PRECISION
vec3 quick_two_sum(vec3 a, vec3 b, out vec3 out_p) {
	vec3 s = a + b;
	out_p = b - (s - a);
	return s;
}
vec3 two_sum(vec3 a, vec3 b, out vec3 out_p) {
	vec3 s = a + b;
	vec3 v = s - a;
	out_p = (a - (s - v)) + (b - v);
	return s;
}
vec3 double_add_vec3(vec3 base_a, vec3 prec_a, vec3 base_b, vec3 prec_b, out vec3 out_precision) {
	vec3 s, t, se, te;
	s = two_sum(base_a, base_b, se);
	t = two_sum(prec_a, prec_b, te);
	se += t;
	s = quick_two_sum(s, se, se);
	se += te;
	s = quick_two_sum(s, se, out_precision);
	return s;
}
#endif
void vertex_shader(vec3 vertex_input,
#ifdef NORMAL_USED
		in vec3 normal_input,
#endif
#ifdef TANGENT_USED
		in vec3 tangent_input,
		in vec3 binormal_input,
#endif
		in uint instance_index, in bool is_multimesh, in uint multimesh_offset, in SceneData scene_data, in mat4 model_matrix, out vec4 screen_pos) {
	vec4 instance_custom = vec4(0.0);
#if defined(COLOR_USED)
	color_interp = color_attrib;
#endif
	mat4 inv_view_matrix = scene_data.inv_view_matrix;
#ifdef USE_DOUBLE_PRECISION
	vec3 model_precision = vec3(model_matrix[0][3], model_matrix[1][3], model_matrix[2][3]);
	model_matrix[0][3] = 0.0;
	model_matrix[1][3] = 0.0;
	model_matrix[2][3] = 0.0;
	vec3 view_precision = vec3(inv_view_matrix[0][3], inv_view_matrix[1][3], inv_view_matrix[2][3]);
	inv_view_matrix[0][3] = 0.0;
	inv_view_matrix[1][3] = 0.0;
	inv_view_matrix[2][3] = 0.0;
#endif
	mat3 model_normal_matrix;
	if (bool(instances.data[instance_index].flags & INSTANCE_FLAGS_NON_UNIFORM_SCALE)) {
		model_normal_matrix = transpose(inverse(mat3(model_matrix)));
	} else {
		model_normal_matrix = mat3(model_matrix);
	}
	mat4 matrix;
	mat4 read_model_matrix = model_matrix;
	if (is_multimesh) {
		
#ifdef USE_PARTICLE_TRAILS
		uint trail_size = (instances.data[instance_index].flags >> INSTANCE_FLAGS_PARTICLE_TRAIL_SHIFT) & INSTANCE_FLAGS_PARTICLE_TRAIL_MASK;
		uint stride = 3 + 1 + 1; 
		uint offset = trail_size * stride * gl_InstanceIndex;
#ifdef COLOR_USED
		vec4 pcolor;
#endif
		{
			uint boffset = offset + bone_attrib.x * stride;
			matrix = mat4(transforms.data[boffset + 0], transforms.data[boffset + 1], transforms.data[boffset + 2], vec4(0.0, 0.0, 0.0, 1.0)) * weight_attrib.x;
#ifdef COLOR_USED
			pcolor = transforms.data[boffset + 3] * weight_attrib.x;
#endif
		}
		if (weight_attrib.y > 0.001) {
			uint boffset = offset + bone_attrib.y * stride;
			matrix += mat4(transforms.data[boffset + 0], transforms.data[boffset + 1], transforms.data[boffset + 2], vec4(0.0, 0.0, 0.0, 1.0)) * weight_attrib.y;
#ifdef COLOR_USED
			pcolor += transforms.data[boffset + 3] * weight_attrib.y;
#endif
		}
		if (weight_attrib.z > 0.001) {
			uint boffset = offset + bone_attrib.z * stride;
			matrix += mat4(transforms.data[boffset + 0], transforms.data[boffset + 1], transforms.data[boffset + 2], vec4(0.0, 0.0, 0.0, 1.0)) * weight_attrib.z;
#ifdef COLOR_USED
			pcolor += transforms.data[boffset + 3] * weight_attrib.z;
#endif
		}
		if (weight_attrib.w > 0.001) {
			uint boffset = offset + bone_attrib.w * stride;
			matrix += mat4(transforms.data[boffset + 0], transforms.data[boffset + 1], transforms.data[boffset + 2], vec4(0.0, 0.0, 0.0, 1.0)) * weight_attrib.w;
#ifdef COLOR_USED
			pcolor += transforms.data[boffset + 3] * weight_attrib.w;
#endif
		}
		instance_custom = transforms.data[offset + 4];
#ifdef COLOR_USED
		color_interp *= pcolor;
#endif
#else
		uint stride = 0;
		{
			
			if (bool(instances.data[instance_index].flags & INSTANCE_FLAGS_MULTIMESH_FORMAT_2D)) {
				stride += 2;
			} else {
				stride += 3;
			}
			if (bool(instances.data[instance_index].flags & INSTANCE_FLAGS_MULTIMESH_HAS_COLOR)) {
				stride += 1;
			}
			if (bool(instances.data[instance_index].flags & INSTANCE_FLAGS_MULTIMESH_HAS_CUSTOM_DATA)) {
				stride += 1;
			}
		}
		uint offset = stride * (gl_InstanceIndex + multimesh_offset);
		if (bool(instances.data[instance_index].flags & INSTANCE_FLAGS_MULTIMESH_FORMAT_2D)) {
			matrix = mat4(transforms.data[offset + 0], transforms.data[offset + 1], vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
			offset += 2;
		} else {
			matrix = mat4(transforms.data[offset + 0], transforms.data[offset + 1], transforms.data[offset + 2], vec4(0.0, 0.0, 0.0, 1.0));
			offset += 3;
		}
		if (bool(instances.data[instance_index].flags & INSTANCE_FLAGS_MULTIMESH_HAS_COLOR)) {
#ifdef COLOR_USED
			color_interp *= transforms.data[offset];
#endif
			offset += 1;
		}
		if (bool(instances.data[instance_index].flags & INSTANCE_FLAGS_MULTIMESH_HAS_CUSTOM_DATA)) {
			instance_custom = transforms.data[offset];
		}
#endif
		
		matrix = transpose(matrix);
#if !defined(USE_DOUBLE_PRECISION) || defined(SKIP_TRANSFORM_USED) || defined(VERTEX_WORLD_COORDS_USED) || defined(MODEL_MATRIX_USED)
		
		
		read_model_matrix = model_matrix * matrix;
#if !defined(USE_DOUBLE_PRECISION) || defined(SKIP_TRANSFORM_USED) || defined(VERTEX_WORLD_COORDS_USED)
		model_matrix = read_model_matrix;
#endif 
#endif 
		model_normal_matrix = model_normal_matrix * mat3(matrix);
	}
	vec3 vertex = vertex_input;
#ifdef NORMAL_USED
	vec3 normal = normal_input;
#endif
#ifdef TANGENT_USED
	vec3 tangent = tangent_input;
	vec3 binormal = binormal_input;
#endif
#ifdef UV_USED
	uv_interp = uv_attrib;
#endif
#if defined(UV2_USED) || defined(USE_LIGHTMAP)
	uv2_interp = uv2_attrib;
#endif
	vec4 uv_scale = instances.data[instance_index].uv_scale;
	if (uv_scale != vec4(0.0)) { 
#ifdef UV_USED
		uv_interp = (uv_interp - 0.5) * uv_scale.xy;
#endif
#if defined(UV2_USED) || defined(USE_LIGHTMAP)
		uv2_interp = (uv2_interp - 0.5) * uv_scale.zw;
#endif
	}
#ifdef OVERRIDE_POSITION
	vec4 position = vec4(1.0);
#endif
#ifdef USE_MULTIVIEW
	mat4 combined_projection = scene_data.projection_matrix;
	mat4 projection_matrix = scene_data.projection_matrix_view[ViewIndex];
	mat4 inv_projection_matrix = scene_data.inv_projection_matrix_view[ViewIndex];
	vec3 eye_offset = scene_data.eye_offset[ViewIndex].xyz;
#else
	mat4 projection_matrix = scene_data.projection_matrix;
	mat4 inv_projection_matrix = scene_data.inv_projection_matrix;
	vec3 eye_offset = vec3(0.0, 0.0, 0.0);
#endif 
#if !defined(SKIP_TRANSFORM_USED) && defined(VERTEX_WORLD_COORDS_USED)
	vertex = (model_matrix * vec4(vertex, 1.0)).xyz;
#ifdef NORMAL_USED
	normal = model_normal_matrix * normal;
#endif
#ifdef TANGENT_USED
	tangent = model_normal_matrix * tangent;
	binormal = model_normal_matrix * binormal;
#endif
#endif
	float roughness = 1.0;
	mat4 modelview = scene_data.view_matrix * model_matrix;
	mat3 modelview_normal = mat3(scene_data.view_matrix) * model_normal_matrix;
	mat4 read_view_matrix = scene_data.view_matrix;
	vec2 read_viewport_size = scene_data.viewport_size;
	{
	}
#if !defined(SKIP_TRANSFORM_USED) && !defined(VERTEX_WORLD_COORDS_USED)
#ifdef USE_DOUBLE_PRECISION
	
	
	
	vec3 model_origin = model_matrix[3].xyz;
	if (is_multimesh) {
		vertex = mat3(matrix) * vertex;
		model_origin = double_add_vec3(model_origin, model_precision, matrix[3].xyz, vec3(0.0), model_precision);
	}
	vertex = mat3(inv_view_matrix * modelview) * vertex;
	vec3 temp_precision; 
	vertex += double_add_vec3(model_origin, model_precision, scene_data.inv_view_matrix[3].xyz, view_precision, temp_precision);
	vertex = mat3(scene_data.view_matrix) * vertex;
#else
	vertex = (modelview * vec4(vertex, 1.0)).xyz;
#endif
#ifdef NORMAL_USED
	normal = modelview_normal * normal;
#endif
#ifdef TANGENT_USED
	binormal = modelview_normal * binormal;
	tangent = modelview_normal * tangent;
#endif
#endif 
#if !defined(SKIP_TRANSFORM_USED) && defined(VERTEX_WORLD_COORDS_USED)
	vertex = (scene_data.view_matrix * vec4(vertex, 1.0)).xyz;
#ifdef NORMAL_USED
	normal = (scene_data.view_matrix * vec4(normal, 0.0)).xyz;
#endif
#ifdef TANGENT_USED
	binormal = (scene_data.view_matrix * vec4(binormal, 0.0)).xyz;
	tangent = (scene_data.view_matrix * vec4(tangent, 0.0)).xyz;
#endif
#endif
	vertex_interp = vertex;
#ifdef NORMAL_USED
	normal_interp = normal;
#endif
#ifdef TANGENT_USED
	tangent_interp = tangent;
	binormal_interp = binormal;
#endif
#ifdef MODE_RENDER_DEPTH
#ifdef MODE_DUAL_PARABOLOID
	vertex_interp.z *= scene_data.dual_paraboloid_side;
	dp_clip = vertex_interp.z; 
	
	vec3 vtx = vertex_interp;
	float distance = length(vtx);
	vtx = normalize(vtx);
	vtx.xy /= 1.0 - vtx.z;
	vtx.z = (distance / scene_data.z_far);
	vtx.z = vtx.z * 2.0 - 1.0;
	vertex_interp = vtx;
#endif
#endif 
#ifdef OVERRIDE_POSITION
	gl_Position = position;
#else
	gl_Position = projection_matrix * vec4(vertex_interp, 1.0);
#endif
#ifdef USE_MULTIVIEW
	combined_projected = combined_projection * vec4(vertex_interp, 1.0);
#endif
#ifdef MOTION_VECTORS
	screen_pos = gl_Position;
#endif
#ifdef MODE_RENDER_DEPTH
	if (scene_data.pancake_shadows) {
		if (gl_Position.z >= 0.9999) {
			gl_Position.z = 0.9999;
		}
	}
#endif
#ifdef MODE_RENDER_MATERIAL
	if (scene_data.material_uv2_mode) {
		vec2 uv_offset = unpackHalf2x16(draw_call.uv_offset);
		gl_Position.xy = (uv2_attrib.xy + uv_offset) * 2.0 - 1.0;
		gl_Position.z = 0.00001;
		gl_Position.w = 1.0;
	}
#endif
}
void _unpack_vertex_attributes(vec4 p_vertex_in, vec3 p_compressed_aabb_position, vec3 p_compressed_aabb_size,
#if defined(NORMAL_USED) || defined(TANGENT_USED)
		vec4 p_normal_in,
#ifdef NORMAL_USED
		out vec3 r_normal,
#endif
		out vec3 r_tangent,
		out vec3 r_binormal,
#endif
		out vec3 r_vertex) {
	r_vertex = p_vertex_in.xyz * p_compressed_aabb_size + p_compressed_aabb_position;
#ifdef NORMAL_USED
	r_normal = oct_to_vec3(p_normal_in.xy * 2.0 - 1.0);
#endif
#if defined(NORMAL_USED) || defined(TANGENT_USED)
	float binormal_sign;
	
	
	if (p_normal_in.z > 0.0 || p_normal_in.w < 1.0) {
		
		vec2 signed_tangent_attrib = p_normal_in.zw * 2.0 - 1.0;
		r_tangent = oct_to_vec3(vec2(signed_tangent_attrib.x, abs(signed_tangent_attrib.y) * 2.0 - 1.0));
		binormal_sign = sign(signed_tangent_attrib.y);
		r_binormal = normalize(cross(r_normal, r_tangent) * binormal_sign);
	} else {
		
		float angle = p_vertex_in.w;
		binormal_sign = angle > 0.5 ? 1.0 : -1.0; 
		angle = abs(angle * 2.0 - 1.0) * M_PI; 
		vec3 axis = r_normal;
		axis_angle_to_tbn(axis, angle, r_tangent, r_binormal, r_normal);
		r_binormal *= binormal_sign;
	}
#endif
}
void main() {
	uint instance_index = draw_call.instance_index;
	bool is_multimesh = bool(instances.data[instance_index].flags & INSTANCE_FLAGS_MULTIMESH);
	if (!is_multimesh) {
		instance_index += gl_InstanceIndex;
	}
	instance_index_interp = instance_index;
	mat4 model_matrix = instances.data[instance_index].transform;
#ifdef MOTION_VECTORS
	
	vec3 prev_vertex;
#ifdef NORMAL_USED
	vec3 prev_normal;
#endif
#if defined(NORMAL_USED) || defined(TANGENT_USED)
	vec3 prev_tangent;
	vec3 prev_binormal;
#endif
	_unpack_vertex_attributes(
			previous_vertex_attrib,
			instances.data[instance_index].compressed_aabb_position_pad.xyz,
			instances.data[instance_index].compressed_aabb_size_pad.xyz,
#if defined(NORMAL_USED) || defined(TANGENT_USED)
			previous_normal_attrib,
#ifdef NORMAL_USED
			prev_normal,
#endif
			prev_tangent,
			prev_binormal,
#endif
			prev_vertex);
	global_time = scene_data_block.prev_data.time;
	vertex_shader(prev_vertex,
#ifdef NORMAL_USED
			prev_normal,
#endif
#ifdef TANGENT_USED
			prev_tangent,
			prev_binormal,
#endif
			instance_index, is_multimesh, draw_call.multimesh_motion_vectors_previous_offset, scene_data_block.prev_data, instances.data[instance_index].prev_transform, prev_screen_position);
#else
	
	vec4 screen_position;
#endif
	vec3 vertex;
#ifdef NORMAL_USED
	vec3 normal;
#endif
#if defined(NORMAL_USED) || defined(TANGENT_USED)
	vec3 tangent;
	vec3 binormal;
#endif
	_unpack_vertex_attributes(
			vertex_angle_attrib,
			instances.data[instance_index].compressed_aabb_position_pad.xyz,
			instances.data[instance_index].compressed_aabb_size_pad.xyz,
#if defined(NORMAL_USED) || defined(TANGENT_USED)
			axis_tangent_attrib,
#ifdef NORMAL_USED
			normal,
#endif
			tangent,
			binormal,
#endif
			vertex);
	
	global_time = scene_data_block.data.time;
	vertex_shader(vertex,
#ifdef NORMAL_USED
			normal,
#endif
#ifdef TANGENT_USED
			tangent,
			binormal,
#endif
			instance_index, is_multimesh, draw_call.multimesh_motion_vectors_current_offset, scene_data_block.data, model_matrix, screen_position);
}
