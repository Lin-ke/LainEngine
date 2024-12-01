#version 450


#define MAX_ROUGHNESS_LOD 7.0

#define USE_RADIANCE_CUBEMAP_ARRAY 

#define SDFGI_OCT_SIZE 1

#define MAX_DIRECTIONAL_LIGHT_DATA_STRUCTS 8

#define MAX_LIGHTMAP_TEXTURES 8

#define MAX_LIGHTMAPS 8

#define MATERIAL_UNIFORM_SET 3

#define FRAGMENT_CODE_USED
#define RENDER_DRIVER_VULKAN
#define M_PI 3.14159265359
#define ROUGHNESS_MAX_LOD 5
#define MAX_VOXEL_GI_INSTANCES 8
#define MAX_VIEWS 2
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
layout(set = 1, binding = 3) uniform textureCubeArray radiance_cubemap;
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


layout(set = 1, binding = 24) uniform texture2D depth_buffer;
layout(set = 1, binding = 25) uniform texture2D color_buffer;
layout(set = 1, binding = 26) uniform texture2D normal_roughness_buffer;
layout(set = 1, binding = 27) uniform texture2D ao_buffer;
layout(set = 1, binding = 28) uniform texture2D ambient_buffer;
layout(set = 1, binding = 29) uniform texture2D reflection_buffer;
#define multiviewSampler sampler2D
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
layout(set = 1, binding = 34) uniform texture2D ssil_buffer;
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
float global_time;
layout(location = 10) out flat uint instance_index_interp;

#define ViewIndex 0
vec2 multiview_uv(vec2 uv) {
	return uv;
}
ivec2 multiview_uv(ivec2 uv) {
	return uv;
}
invariant gl_Position;

void vertex_shader(vec3 vertex_input,

		in uint instance_index, in bool is_multimesh, in uint multimesh_offset, in SceneData scene_data, in mat4 model_matrix, out vec4 screen_pos) {
	vec4 instance_custom = vec4(0.0);

	mat4 inv_view_matrix = scene_data.inv_view_matrix;
	mat3 model_normal_matrix;
	if (bool(instances.data[instance_index].flags & INSTANCE_FLAGS_NON_UNIFORM_SCALE)) {
		model_normal_matrix = transpose(inverse(mat3(model_matrix)));
	} else {
		model_normal_matrix = mat3(model_matrix);
	}
	mat4 matrix;
	mat4 read_model_matrix = model_matrix;
	if (is_multimesh) {
		matrix = transpose(matrix);
		model_normal_matrix = model_normal_matrix * mat3(matrix);
	}
	vec3 vertex = vertex_input;
	vec4 uv_scale = instances.data[instance_index].uv_scale;
	if (uv_scale != vec4(0.0)) { 
	}
	mat4 projection_matrix = scene_data.projection_matrix;
	mat4 inv_projection_matrix = scene_data.inv_projection_matrix;
	vec3 eye_offset = vec3(0.0, 0.0, 0.0);
	float roughness = 1.0;
	mat4 modelview = scene_data.view_matrix * model_matrix;
	mat3 modelview_normal = mat3(scene_data.view_matrix) * model_normal_matrix;
	mat4 read_view_matrix = scene_data.view_matrix;
	vec2 read_viewport_size = scene_data.viewport_size;
	{
	}
	vertex = (modelview * vec4(vertex, 1.0)).xyz;
	vertex_interp = vertex;
	gl_Position = projection_matrix * vec4(vertex_interp, 1.0);
}
void _unpack_vertex_attributes(vec4 p_vertex_in, vec3 p_compressed_aabb_position, vec3 p_compressed_aabb_size,
		out vec3 r_vertex) {
	r_vertex = p_vertex_in.xyz * p_compressed_aabb_size + p_compressed_aabb_position;

}
void main() {
	uint instance_index = draw_call.instance_index;
	bool is_multimesh = bool(instances.data[instance_index].flags & INSTANCE_FLAGS_MULTIMESH);
	if (!is_multimesh) {
		instance_index += gl_InstanceIndex;
	}
	instance_index_interp = instance_index;
	mat4 model_matrix = instances.data[instance_index].transform;
	vec4 screen_position;
	vec3 vertex;
	_unpack_vertex_attributes(
			vertex_angle_attrib,
			instances.data[instance_index].compressed_aabb_position_pad.xyz,
			instances.data[instance_index].compressed_aabb_size_pad.xyz,

			vertex);
	
	global_time = scene_data_block.data.time;
	vertex_shader(vertex,
			instance_index, is_multimesh, draw_call.multimesh_motion_vectors_current_offset, scene_data_block.data, model_matrix, screen_position);
}
