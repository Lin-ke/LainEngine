#pragma once
#ifndef __RENDERING_SYSTEM_H__
#define __RENDERING_SYSTEM_H__
#include "base.h"
#include "core/io/image.h"
#include "core/meta/class_db.h"
#include "function/display/window_system.h"
#include "function/render/rendering_device/rendering_device.h"
namespace lain {
// 通过renderingsystem default 继承，其实现通过各个自子server （例如shader 就是 material_storage server 实现。）
class RenderingSystem : public Object {
  LCLASS(RenderingSystem, Object);

 public:
 // Constants
	 	enum {
		NO_INDEX_ARRAY = -1,
		ARRAY_WEIGHTS_SIZE = 4,
		CANVAS_ITEM_Z_MIN = -4096,
		CANVAS_ITEM_Z_MAX = 4096,
		MAX_GLOW_LEVELS = 7,
		MAX_CURSORS = 8,
		MAX_2D_DIRECTIONAL_LIGHTS = 8,
		MAX_MESH_SURFACES = 256
	};

  RenderingSystem() { p_singleton = this; };
  virtual ~RenderingSystem() { p_singleton = nullptr; }
  L_INLINE static RenderingSystem* get_singleton() { return p_singleton; }
  bool is_render_loop_enabled() const { return render_loop_enabled; }
  void set_render_loop_enabled(bool p_enabled) { render_loop_enabled = p_enabled; }
	bool is_low_end() const {return false;}
 private:
  static RenderingSystem* p_singleton;
  bool render_loop_enabled = true;

 public:
  /********TEXTURE ******* */
  /********TEXTURE******* */
  /********TEXTURE******* */
	enum TextureLayeredType {
		TEXTURE_LAYERED_2D_ARRAY,
		TEXTURE_LAYERED_CUBEMAP,
		TEXTURE_LAYERED_CUBEMAP_ARRAY,
	};
  enum CubeMapLayer {
		CUBEMAP_LAYER_LEFT,
		CUBEMAP_LAYER_RIGHT,
		CUBEMAP_LAYER_BOTTOM,
		CUBEMAP_LAYER_TOP,
		CUBEMAP_LAYER_FRONT,
		CUBEMAP_LAYER_BACK
	};
	virtual RID texture_2d_create(const Ref<Image> &p_image) = 0;
	virtual RID texture_2d_layered_create(const Vector<Ref<Image>> &p_layers, TextureLayeredType p_layered_type) = 0;
	virtual RID texture_3d_create(Image::Format, int p_width, int p_height, int p_depth, bool p_mipmaps, const Vector<Ref<Image>> &p_data) = 0; //all slices, then all the mipmaps, must be coherent
 	virtual Ref<Image> texture_2d_get(RID p_texture) const = 0;
	virtual Ref<Image> texture_2d_layer_get(RID p_texture, int p_layer) const = 0;
	virtual Vector<Ref<Image>> texture_3d_get(RID p_texture) const = 0;

	virtual void texture_2d_update(RID p_texture, const Ref<Image> &p_image, int p_layer = 0) = 0;
	virtual void texture_3d_update(RID p_texture, const Vector<Ref<Image>> &p_data) = 0;

	virtual RID texture_rd_create(const RID &p_rd_texture, const RenderingSystem::TextureLayeredType p_layer_type = RenderingSystem::TEXTURE_LAYERED_2D_ARRAY) = 0;
	virtual RID texture_get_rd_texture(RID p_texture, bool p_srgb = false) const = 0;
	virtual uint64_t texture_get_native_handle(RID p_texture, bool p_srgb = false) const = 0;

  /********SHADER ******* */
  /********SHADER ******* */
  /********SHADER ******* */
  enum ShaderMode { SHADER_SPATIAL, SHADER_CANVAS_ITEM, SHADER_PARTICLES, SHADER_SKY, SHADER_FOG, SHADER_MAX };

  virtual RID shader_create() = 0;

  virtual void shader_set_code(RID p_shader, const String& p_code) = 0;
  virtual void shader_set_path_hint(RID p_shader, const String& p_path) = 0;
  virtual String shader_get_code(RID p_shader) const = 0;
  virtual void get_shader_parameter_list(RID p_shader, List<PropertyInfo>* p_param_list) const = 0;
  virtual Variant shader_get_parameter_default(RID p_shader, const StringName& p_param) const = 0;

  virtual void shader_set_default_texture_parameter(RID p_shader, const StringName& p_name, RID p_texture, int p_index) = 0;
  virtual RID shader_get_default_texture_parameter(RID p_shader, const StringName& p_name, int p_index) const = 0;
	
  struct ShaderNativeSourceCode {
		struct Version {
			struct Stage {
				String name;
				String code;
			};
			Vector<Stage> stages;
		};
		Vector<Version> versions;
	};
	virtual ShaderNativeSourceCode shader_get_native_source_code(RID p_shader) const = 0;
	/* COMMON MATERIAL API */
	/* COMMON MATERIAL API */
	/* COMMON MATERIAL API */
	enum {
		MATERIAL_RENDER_PRIORITY_MIN = -128,
		MATERIAL_RENDER_PRIORITY_MAX = 127,
	};

	virtual RID material_create() = 0;

	virtual void material_set_shader(RID p_shader_material, RID p_shader) = 0;

	virtual void material_set_param(RID p_material, const StringName &p_param, const Variant &p_value) = 0;
	virtual Variant material_get_param(RID p_material, const StringName &p_param) const = 0;

	virtual void material_set_render_priority(RID p_material, int priority) = 0;

	virtual void material_set_next_pass(RID p_material, RID p_next_material) = 0;
 
 	/* GLOBAL SHADER UNIFORMS */

	// enum GlobalShaderParameterType {
	// 	GLOBAL_VAR_TYPE_BOOL,
	// 	GLOBAL_VAR_TYPE_BVEC2,
	// 	GLOBAL_VAR_TYPE_BVEC3,
	// 	GLOBAL_VAR_TYPE_BVEC4,
	// 	GLOBAL_VAR_TYPE_INT,
	// 	GLOBAL_VAR_TYPE_IVEC2,
	// 	GLOBAL_VAR_TYPE_IVEC3,
	// 	GLOBAL_VAR_TYPE_IVEC4,
	// 	GLOBAL_VAR_TYPE_RECT2I,
	// 	GLOBAL_VAR_TYPE_UINT,
	// 	GLOBAL_VAR_TYPE_UVEC2,
	// 	GLOBAL_VAR_TYPE_UVEC3,
	// 	GLOBAL_VAR_TYPE_UVEC4,
	// 	GLOBAL_VAR_TYPE_FLOAT,
	// 	GLOBAL_VAR_TYPE_VEC2,
	// 	GLOBAL_VAR_TYPE_VEC3,
	// 	GLOBAL_VAR_TYPE_VEC4,
	// 	GLOBAL_VAR_TYPE_COLOR,
	// 	GLOBAL_VAR_TYPE_RECT2,
	// 	GLOBAL_VAR_TYPE_MAT2,
	// 	GLOBAL_VAR_TYPE_MAT3,
	// 	GLOBAL_VAR_TYPE_MAT4,
	// 	GLOBAL_VAR_TYPE_TRANSFORM_2D,
	// 	GLOBAL_VAR_TYPE_TRANSFORM,
	// 	GLOBAL_VAR_TYPE_SAMPLER2D,
	// 	GLOBAL_VAR_TYPE_SAMPLER2DARRAY,
	// 	GLOBAL_VAR_TYPE_SAMPLER3D,
	// 	GLOBAL_VAR_TYPE_SAMPLERCUBE,
	// 	GLOBAL_VAR_TYPE_MAX
	// };

	// virtual void global_shader_parameter_add(const StringName &p_name, GlobalShaderParameterType p_type, const Variant &p_value) = 0;
	// virtual void global_shader_parameter_remove(const StringName &p_name) = 0;
	// virtual Vector<StringName> global_shader_parameter_get_list() const = 0;

	// virtual void global_shader_parameter_set(const StringName &p_name, const Variant &p_value) = 0;
	// virtual void global_shader_parameter_set_override(const StringName &p_name, const Variant &p_value) = 0;

	// virtual Variant global_shader_parameter_get(const StringName &p_name) const = 0;
	// virtual GlobalShaderParameterType global_shader_parameter_get_type(const StringName &p_name) const = 0;

	// virtual void global_shader_parameters_load_settings(bool p_load_textures) = 0;
	// virtual void global_shader_parameters_clear() = 0;

	// static int global_shader_uniform_type_get_shader_datatype(GlobalShaderParameterType p_type);


  /******************
     * MESH API
     ******************/
  	/* MESH API */

	enum ArrayType {
		ARRAY_VERTEX = 0, // RG32F (2D), RGB32F, RGBA16 (compressed)
		ARRAY_NORMAL = 1, // RG16
		ARRAY_TANGENT = 2, // BA16 (with normal) or A16 (with vertex, when compressed)
		ARRAY_COLOR = 3, // RGBA8
		ARRAY_TEX_UV = 4, // RG32F or RG16
		ARRAY_TEX_UV2 = 5, // RG32F or RG16
		ARRAY_CUSTOM0 = 6, // Depends on ArrayCustomFormat.
		ARRAY_CUSTOM1 = 7,
		ARRAY_CUSTOM2 = 8,
		ARRAY_CUSTOM3 = 9,
		ARRAY_BONES = 10, // RGBA16UI (x2 if 8 weights)
		ARRAY_WEIGHTS = 11, // RGBA16UNORM (x2 if 8 weights)
		ARRAY_INDEX = 12, // 16 or 32 bits depending on length > 0xFFFF.
		ARRAY_MAX = 13
	};

	enum {
		ARRAY_CUSTOM_COUNT = ARRAY_BONES - ARRAY_CUSTOM0
	};

	enum ArrayCustomFormat {
		ARRAY_CUSTOM_RGBA8_UNORM,
		ARRAY_CUSTOM_RGBA8_SNORM,
		ARRAY_CUSTOM_RG_HALF,
		ARRAY_CUSTOM_RGBA_HALF,
		ARRAY_CUSTOM_R_FLOAT,
		ARRAY_CUSTOM_RG_FLOAT,
		ARRAY_CUSTOM_RGB_FLOAT,
		ARRAY_CUSTOM_RGBA_FLOAT,
		ARRAY_CUSTOM_MAX
	};

	enum ArrayFormat : uint64_t {
		/* ARRAY FORMAT FLAGS */
		ARRAY_FORMAT_VERTEX = 1 << ARRAY_VERTEX,
		ARRAY_FORMAT_NORMAL = 1 << ARRAY_NORMAL,
		ARRAY_FORMAT_TANGENT = 1 << ARRAY_TANGENT,
		ARRAY_FORMAT_COLOR = 1 << ARRAY_COLOR,
		ARRAY_FORMAT_TEX_UV = 1 << ARRAY_TEX_UV,
		ARRAY_FORMAT_TEX_UV2 = 1 << ARRAY_TEX_UV2,
		ARRAY_FORMAT_CUSTOM0 = 1 << ARRAY_CUSTOM0,
		ARRAY_FORMAT_CUSTOM1 = 1 << ARRAY_CUSTOM1,
		ARRAY_FORMAT_CUSTOM2 = 1 << ARRAY_CUSTOM2,
		ARRAY_FORMAT_CUSTOM3 = 1 << ARRAY_CUSTOM3,
		ARRAY_FORMAT_BONES = 1 << ARRAY_BONES,
		ARRAY_FORMAT_WEIGHTS = 1 << ARRAY_WEIGHTS,
		ARRAY_FORMAT_INDEX = 1 << ARRAY_INDEX,

		ARRAY_FORMAT_BLEND_SHAPE_MASK = ARRAY_FORMAT_VERTEX | ARRAY_FORMAT_NORMAL | ARRAY_FORMAT_TANGENT,

		ARRAY_FORMAT_CUSTOM_BASE = (ARRAY_INDEX + 1),
		ARRAY_FORMAT_CUSTOM_BITS = 3,
		ARRAY_FORMAT_CUSTOM_MASK = 0x7,
		ARRAY_FORMAT_CUSTOM0_SHIFT = (ARRAY_FORMAT_CUSTOM_BASE + 0),
		ARRAY_FORMAT_CUSTOM1_SHIFT = (ARRAY_FORMAT_CUSTOM_BASE + ARRAY_FORMAT_CUSTOM_BITS),
		ARRAY_FORMAT_CUSTOM2_SHIFT = (ARRAY_FORMAT_CUSTOM_BASE + ARRAY_FORMAT_CUSTOM_BITS * 2),
		ARRAY_FORMAT_CUSTOM3_SHIFT = (ARRAY_FORMAT_CUSTOM_BASE + ARRAY_FORMAT_CUSTOM_BITS * 3),

		ARRAY_COMPRESS_FLAGS_BASE = (ARRAY_INDEX + 1 + 12),

		ARRAY_FLAG_USE_2D_VERTICES = 1 << (ARRAY_COMPRESS_FLAGS_BASE + 0),
		ARRAY_FLAG_USE_DYNAMIC_UPDATE = 1 << (ARRAY_COMPRESS_FLAGS_BASE + 1),
		ARRAY_FLAG_USE_8_BONE_WEIGHTS = 1 << (ARRAY_COMPRESS_FLAGS_BASE + 2),

		ARRAY_FLAG_USES_EMPTY_VERTEX_ARRAY = 1 << (ARRAY_COMPRESS_FLAGS_BASE + 3),

		ARRAY_FLAG_COMPRESS_ATTRIBUTES = 1 << (ARRAY_COMPRESS_FLAGS_BASE + 4),
		// We leave enough room for up to 5 more compression flags.

		ARRAY_FLAG_FORMAT_VERSION_BASE = ARRAY_COMPRESS_FLAGS_BASE + 10,
		ARRAY_FLAG_FORMAT_VERSION_SHIFT = ARRAY_FLAG_FORMAT_VERSION_BASE,
		// When changes are made to the mesh format, add a new version and use it for the CURRENT_VERSION.
		ARRAY_FLAG_FORMAT_VERSION_1 = 0,
		ARRAY_FLAG_FORMAT_VERSION_2 = 1ULL << ARRAY_FLAG_FORMAT_VERSION_SHIFT,
		ARRAY_FLAG_FORMAT_CURRENT_VERSION = ARRAY_FLAG_FORMAT_VERSION_2,
		ARRAY_FLAG_FORMAT_VERSION_MASK = 0xFF, // 8 bits version
	};

  enum BlendShapeMode {
    BLEND_SHAPE_MODE_NORMALIZED,
    BLEND_SHAPE_MODE_RELATIVE,
  };

  enum PrimitiveType {
    PRIMITIVE_POINTS,
    PRIMITIVE_LINES,
    PRIMITIVE_LINE_STRIP,
    PRIMITIVE_TRIANGLES,
    PRIMITIVE_TRIANGLE_STRIP,
    PRIMITIVE_MAX,
  };
	// 大家都使用的数据类，在不同后端中被翻译，添加
	// 填写这个surface data 就可以 用来添加到mesh里了。

  struct SurfaceData {
    PrimitiveType primitive = PRIMITIVE_MAX;
    uint64_t format = ARRAY_FLAG_FORMAT_CURRENT_VERSION; // @todo 这里可以用bitfield
    Vector<uint8_t> vertex_data;     // Vertex, Normal, Tangent (change with skinning, blendshape).
    Vector<uint8_t> attribute_data;  // Color, UV, UV2, Custom0-3.
    Vector<uint8_t> skin_data;       // Bone index, Bone weight.
    uint32_t vertex_count = 0;
    Vector<uint8_t> index_data;
    uint32_t index_count = 0;

    AABB aabb;
    struct LOD {
      float edge_length = 0.0f;
      Vector<uint8_t> index_data;
    };
    Vector<LOD> lods;
    Vector<AABB> bone_aabbs;

    // Transforms used in runtime bone AABBs compute.
    // Since bone AABBs is saved in Mesh space, but bones is in Skeleton space.
    Transform3D mesh_to_skeleton_xform;

    Vector<uint8_t> blend_shape_data;

    Vector4 uv_scale;

    RID material;
  };

	enum MultimeshTransformFormat {
		MULTIMESH_TRANSFORM_2D,
		MULTIMESH_TRANSFORM_3D,
	};

	/* SCENARIO API */

	virtual RID scenario_create() = 0;

	virtual void scenario_set_environment(RID p_scenario, RID p_environment) = 0;
	virtual void scenario_set_fallback_environment(RID p_scenario, RID p_environment) = 0;
	virtual void scenario_set_camera_attributes(RID p_scenario, RID p_camera_attributes) = 0;
	virtual void scenario_set_compositor(RID p_scenario, RID p_compositor) = 0;

  /**************** */
  /* INSTANCING API */
  /**************** */

  enum InstanceType {
    INSTANCE_NONE,
    INSTANCE_MESH,
    INSTANCE_MULTIMESH,
    INSTANCE_PARTICLES,
    INSTANCE_PARTICLES_COLLISION,
    INSTANCE_LIGHT,
    INSTANCE_REFLECTION_PROBE,
    INSTANCE_DECAL,
    INSTANCE_VOXEL_GI,
    INSTANCE_LIGHTMAP,
    INSTANCE_OCCLUDER,
    INSTANCE_VISIBLITY_NOTIFIER,
    INSTANCE_FOG_VOLUME,
    INSTANCE_MAX,

    INSTANCE_GEOMETRY_MASK = (1 << INSTANCE_MESH) | (1 << INSTANCE_MULTIMESH) | (1 << INSTANCE_PARTICLES)
  };

		enum InstanceFlags {
		INSTANCE_FLAG_USE_BAKED_LIGHT,
		INSTANCE_FLAG_USE_DYNAMIC_GI,
		INSTANCE_FLAG_DRAW_NEXT_FRAME_IF_VISIBLE,
		INSTANCE_FLAG_IGNORE_OCCLUSION_CULLING,
		INSTANCE_FLAG_MAX
	};
		enum VisibilityRangeFadeMode {
		VISIBILITY_RANGE_FADE_DISABLED,
		VISIBILITY_RANGE_FADE_SELF,
		VISIBILITY_RANGE_FADE_DEPENDENCIES,
	};

	virtual RID instance_create() = 0;
	virtual void instance_set_base(RID p_instance, RID p_base) = 0;
	virtual void instance_set_scenario(RID p_instance, RID p_scenario) = 0;
	virtual void instance_set_layer_mask(RID p_instance, uint32_t p_mask) = 0;
	virtual void instance_set_pivot_data(RID p_instance, float p_sorting_offset, bool p_use_aabb_center) = 0;
	virtual void instance_set_transform(RID p_instance, const Transform3D &p_transform) = 0;
	virtual void instance_attach_object_instance_id(RID p_instance, ObjectID p_id) = 0;
	virtual void instance_set_blend_shape_weight(RID p_instance, int p_shape, float p_weight) = 0;
	virtual void instance_set_surface_override_material(RID p_instance, int p_surface, RID p_material) = 0;
	virtual void instance_set_visible(RID p_instance, bool p_visible) = 0;

	virtual void instance_set_custom_aabb(RID p_instance, AABB aabb) = 0;

	virtual void instance_attach_skeleton(RID p_instance, RID p_skeleton) = 0;

	virtual void instance_set_extra_visibility_margin(RID p_instance, real_t p_margin) = 0;
	virtual void instance_set_visibility_parent(RID p_instance, RID p_parent_instance) = 0;

	virtual void instance_set_ignore_culling(RID p_instance, bool p_enabled) = 0;
	
	virtual void instance_geometry_set_material_override(RID p_instance, RID p_material) = 0;
	virtual void instance_geometry_set_lightmap(RID p_instance, RID p_lightmap,const Rect2 &p_lightmap_uv_scale, int p_slice_index ) = 0;
	virtual void instance_geometry_set_material_overlay(RID p_instance, RID p_material) = 0;
    /// *************** ///
  /// ***CAMERA API*** ///
  /// *************** ///
  	virtual RID camera_create() = 0;
	virtual void camera_set_perspective(RID p_camera, float p_fovy_degrees, float p_z_near, float p_z_far) = 0;
	virtual void camera_set_orthogonal(RID p_camera, float p_size, float p_z_near, float p_z_far) = 0;
	virtual void camera_set_frustum(RID p_camera, float p_size, Vector2 p_offset, float p_z_near, float p_z_far) = 0;
	virtual void camera_set_transform(RID p_camera, const Transform3D &p_transform) = 0;
	virtual void camera_set_cull_mask(RID p_camera, uint32_t p_layers) = 0;
	virtual void camera_set_environment(RID p_camera, RID p_env) = 0;
	virtual void camera_set_camera_attributes(RID p_camera, RID p_camera_attributes) = 0;
	virtual void camera_set_compositor(RID p_camera, RID p_compositor) = 0;
	virtual void camera_set_use_vertical_aspect(RID p_camera, bool p_enable) = 0;
  
  /// *************** ///
  /// ***VIEWPORT API*** ///
  /// *************** ///
	// sampler使用的
		enum CanvasItemTextureFilter {
		CANVAS_ITEM_TEXTURE_FILTER_DEFAULT, // Uses canvas item setting for draw command, uses global setting for canvas item.
		CANVAS_ITEM_TEXTURE_FILTER_NEAREST,
		CANVAS_ITEM_TEXTURE_FILTER_LINEAR,
		CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS,
		CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS,
		CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS_ANISOTROPIC,
		CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS_ANISOTROPIC,
		CANVAS_ITEM_TEXTURE_FILTER_MAX
	};

	enum CanvasItemTextureRepeat {
		CANVAS_ITEM_TEXTURE_REPEAT_DEFAULT, // Uses canvas item setting for draw command, uses global setting for canvas item.
		CANVAS_ITEM_TEXTURE_REPEAT_DISABLED,
		CANVAS_ITEM_TEXTURE_REPEAT_ENABLED,
		CANVAS_ITEM_TEXTURE_REPEAT_MIRROR,
		CANVAS_ITEM_TEXTURE_REPEAT_MAX,
	};
	virtual RID viewport_create() = 0;

	enum ViewportScaling3DMode {
		VIEWPORT_SCALING_3D_MODE_BILINEAR,
		VIEWPORT_SCALING_3D_MODE_FSR,
		VIEWPORT_SCALING_3D_MODE_FSR2,
		VIEWPORT_SCALING_3D_MODE_MAX,
		VIEWPORT_SCALING_3D_MODE_OFF = 255, // for internal use only
	};
	virtual void viewport_set_size(RID p_viewport, int p_width, int p_height) = 0;
	virtual void viewport_set_parent_viewport(RID p_viewport, RID p_parent_viewport) = 0;
	virtual void viewport_attach_to_screen(RID p_viewport, const Rect2 &p_rect = Rect2(), WindowSystem::WindowID p_screen = WindowSystem::MAIN_WINDOW_ID) = 0;
	virtual void viewport_set_render_direct_to_screen(RID p_viewport, bool p_enable) = 0;
	virtual void viewport_set_active(RID p_viewport, bool p_active) = 0;
  enum ViewportUpdateMode {
		VIEWPORT_UPDATE_DISABLED,
		VIEWPORT_UPDATE_ONCE, // Then goes to disabled, must be manually updated.
		VIEWPORT_UPDATE_WHEN_VISIBLE, // Default
		VIEWPORT_UPDATE_WHEN_PARENT_VISIBLE,
		VIEWPORT_UPDATE_ALWAYS
	};
	virtual void viewport_set_update_mode(RID p_viewport, ViewportUpdateMode p_mode) = 0;
	virtual ViewportUpdateMode viewport_get_update_mode(RID p_viewport) const = 0;


	enum ViewportClearMode {
		VIEWPORT_CLEAR_ALWAYS,
		VIEWPORT_CLEAR_NEVER,
		VIEWPORT_CLEAR_ONLY_NEXT_FRAME
	};
	virtual void viewport_set_clear_mode(RID p_viewport, ViewportClearMode p_clear_mode) = 0;
	virtual RID viewport_get_render_target(RID p_viewport) const = 0;
	virtual RID viewport_get_texture(RID p_viewport) const = 0;

	virtual void viewport_attach_camera(RID p_viewport, RID p_camera) = 0;
	virtual void viewport_set_scenario(RID p_viewport, RID p_scenario) = 0;
	virtual void viewport_attach_canvas(RID p_viewport, RID p_canvas) = 0;
	enum ViewportMSAA {
		VIEWPORT_MSAA_DISABLED,
		VIEWPORT_MSAA_2X,
		VIEWPORT_MSAA_4X,
		VIEWPORT_MSAA_8X,
		VIEWPORT_MSAA_MAX,
	};
	virtual void viewport_set_msaa_3d(RID p_viewport, ViewportMSAA p_msaa) = 0;
	virtual void viewport_set_msaa_2d(RID p_viewport, ViewportMSAA p_msaa) = 0;


	enum ViewportEnvironmentMode {
		VIEWPORT_ENVIRONMENT_DISABLED,
		VIEWPORT_ENVIRONMENT_ENABLED,
		VIEWPORT_ENVIRONMENT_INHERIT,
		VIEWPORT_ENVIRONMENT_MAX,
	};

	enum ViewportScreenSpaceAA {
		VIEWPORT_SCREEN_SPACE_AA_DISABLED,
		VIEWPORT_SCREEN_SPACE_AA_FXAA,
		VIEWPORT_SCREEN_SPACE_AA_MAX,
	};

		enum ViewportOcclusionCullingBuildQuality {
		VIEWPORT_OCCLUSION_BUILD_QUALITY_LOW = 0,
		VIEWPORT_OCCLUSION_BUILD_QUALITY_MEDIUM = 1,
		VIEWPORT_OCCLUSION_BUILD_QUALITY_HIGH = 2,
	};
	enum ViewportRenderInfo {
		VIEWPORT_RENDER_INFO_OBJECTS_IN_FRAME,
		VIEWPORT_RENDER_INFO_PRIMITIVES_IN_FRAME,
		VIEWPORT_RENDER_INFO_DRAW_CALLS_IN_FRAME,
		VIEWPORT_RENDER_INFO_MAX,
	};
		enum ViewportRenderInfoType {
		VIEWPORT_RENDER_INFO_TYPE_VISIBLE,
		VIEWPORT_RENDER_INFO_TYPE_SHADOW,
		VIEWPORT_RENDER_INFO_TYPE_CANVAS,
		VIEWPORT_RENDER_INFO_TYPE_MAX
	};

		enum ViewportDebugDraw {
		VIEWPORT_DEBUG_DRAW_DISABLED,
		VIEWPORT_DEBUG_DRAW_UNSHADED,
		VIEWPORT_DEBUG_DRAW_LIGHTING,
		VIEWPORT_DEBUG_DRAW_OVERDRAW,
		VIEWPORT_DEBUG_DRAW_WIREFRAME,
		VIEWPORT_DEBUG_DRAW_NORMAL_BUFFER,
		VIEWPORT_DEBUG_DRAW_VOXEL_GI_ALBEDO,
		VIEWPORT_DEBUG_DRAW_VOXEL_GI_LIGHTING,
		VIEWPORT_DEBUG_DRAW_VOXEL_GI_EMISSION,
		VIEWPORT_DEBUG_DRAW_SHADOW_ATLAS,
		VIEWPORT_DEBUG_DRAW_DIRECTIONAL_SHADOW_ATLAS,
		VIEWPORT_DEBUG_DRAW_SCENE_LUMINANCE,
		VIEWPORT_DEBUG_DRAW_SSAO,
		VIEWPORT_DEBUG_DRAW_SSIL,
		VIEWPORT_DEBUG_DRAW_PSSM_SPLITS,
		VIEWPORT_DEBUG_DRAW_DECAL_ATLAS,
		VIEWPORT_DEBUG_DRAW_SDFGI,
		VIEWPORT_DEBUG_DRAW_SDFGI_PROBES,
		VIEWPORT_DEBUG_DRAW_GI_BUFFER,
		VIEWPORT_DEBUG_DRAW_DISABLE_LOD,
		VIEWPORT_DEBUG_DRAW_CLUSTER_OMNI_LIGHTS,
		VIEWPORT_DEBUG_DRAW_CLUSTER_SPOT_LIGHTS,
		VIEWPORT_DEBUG_DRAW_CLUSTER_DECALS,
		VIEWPORT_DEBUG_DRAW_CLUSTER_REFLECTION_PROBES,
		VIEWPORT_DEBUG_DRAW_OCCLUDERS,
		VIEWPORT_DEBUG_DRAW_MOTION_VECTORS,
		VIEWPORT_DEBUG_DRAW_INTERNAL_BUFFER,
	};
  /// *************** ///
  /// ***DECAL API*** /// 贴花
  /// *************** ///
	enum DecalFilter {
		DECAL_FILTER_NEAREST,
		DECAL_FILTER_LINEAR,
		DECAL_FILTER_NEAREST_MIPMAPS,
		DECAL_FILTER_LINEAR_MIPMAPS,
		DECAL_FILTER_NEAREST_MIPMAPS_ANISOTROPIC,
		DECAL_FILTER_LINEAR_MIPMAPS_ANISOTROPIC,
	};

  	enum LightProjectorFilter {
		LIGHT_PROJECTOR_FILTER_NEAREST,
		LIGHT_PROJECTOR_FILTER_LINEAR,
		LIGHT_PROJECTOR_FILTER_NEAREST_MIPMAPS,
		LIGHT_PROJECTOR_FILTER_LINEAR_MIPMAPS,
		LIGHT_PROJECTOR_FILTER_NEAREST_MIPMAPS_ANISOTROPIC,
		LIGHT_PROJECTOR_FILTER_LINEAR_MIPMAPS_ANISOTROPIC,
	};


	/* GLOBAL SHADER UNIFORMS */

	enum GlobalShaderParameterType {
		GLOBAL_VAR_TYPE_BOOL,
		GLOBAL_VAR_TYPE_BVEC2,
		GLOBAL_VAR_TYPE_BVEC3,
		GLOBAL_VAR_TYPE_BVEC4,
		GLOBAL_VAR_TYPE_INT,
		GLOBAL_VAR_TYPE_IVEC2,
		GLOBAL_VAR_TYPE_IVEC3,
		GLOBAL_VAR_TYPE_IVEC4,
		GLOBAL_VAR_TYPE_RECT2I,
		GLOBAL_VAR_TYPE_UINT,
		GLOBAL_VAR_TYPE_UVEC2,
		GLOBAL_VAR_TYPE_UVEC3,
		GLOBAL_VAR_TYPE_UVEC4,
		GLOBAL_VAR_TYPE_FLOAT,
		GLOBAL_VAR_TYPE_VEC2,
		GLOBAL_VAR_TYPE_VEC3,
		GLOBAL_VAR_TYPE_VEC4,
		GLOBAL_VAR_TYPE_COLOR,
		GLOBAL_VAR_TYPE_RECT2,
		GLOBAL_VAR_TYPE_MAT2,
		GLOBAL_VAR_TYPE_MAT3,
		GLOBAL_VAR_TYPE_MAT4,
		GLOBAL_VAR_TYPE_TRANSFORM_2D,
		GLOBAL_VAR_TYPE_TRANSFORM,
		GLOBAL_VAR_TYPE_SAMPLER2D,
		GLOBAL_VAR_TYPE_SAMPLER2DARRAY,
		GLOBAL_VAR_TYPE_SAMPLER3D,
		GLOBAL_VAR_TYPE_SAMPLERCUBE,
		GLOBAL_VAR_TYPE_MAX
	};

	
	/* Light API */
	/* Light API */
	/* Light API */
	// 光源类型： 平行光，点光源，聚光灯
	// 反射探针
	// 阴影图集
	enum LightType {
		LIGHT_DIRECTIONAL,
		LIGHT_OMNI,
		LIGHT_SPOT
	};

	enum LightParam {
		LIGHT_PARAM_ENERGY,
		LIGHT_PARAM_INDIRECT_ENERGY,
		LIGHT_PARAM_VOLUMETRIC_FOG_ENERGY,
		LIGHT_PARAM_SPECULAR,
		LIGHT_PARAM_RANGE,
		LIGHT_PARAM_SIZE,
		LIGHT_PARAM_ATTENUATION,
		LIGHT_PARAM_SPOT_ANGLE,
		LIGHT_PARAM_SPOT_ATTENUATION,
		LIGHT_PARAM_SHADOW_MAX_DISTANCE,
		LIGHT_PARAM_SHADOW_SPLIT_1_OFFSET,
		LIGHT_PARAM_SHADOW_SPLIT_2_OFFSET,
		LIGHT_PARAM_SHADOW_SPLIT_3_OFFSET,
		LIGHT_PARAM_SHADOW_FADE_START,
		LIGHT_PARAM_SHADOW_NORMAL_BIAS,
		LIGHT_PARAM_SHADOW_BIAS,
		LIGHT_PARAM_SHADOW_PANCAKE_SIZE,
		LIGHT_PARAM_SHADOW_OPACITY,
		LIGHT_PARAM_SHADOW_BLUR,
		LIGHT_PARAM_TRANSMITTANCE_BIAS,
		LIGHT_PARAM_INTENSITY,
		LIGHT_PARAM_MAX
	};

	virtual RID directional_light_create() = 0;
	virtual RID omni_light_create() = 0;
	virtual RID spot_light_create() = 0;

	virtual void light_set_color(RID p_light, const Color &p_color) = 0;
	virtual void light_set_param(RID p_light, LightParam p_param, float p_value) = 0;
	virtual void light_set_shadow(RID p_light, bool p_enabled) = 0;
	virtual void light_set_projector(RID p_light, RID p_texture) = 0;
	virtual void light_set_negative(RID p_light, bool p_enable) = 0;
	virtual void light_set_cull_mask(RID p_light, uint32_t p_mask) = 0;
	virtual void light_set_distance_fade(RID p_light, bool p_enabled, float p_begin, float p_shadow, float p_length) = 0;
	virtual void light_set_reverse_cull_face_mode(RID p_light, bool p_enabled) = 0;

	enum LightBakeMode {
			LIGHT_BAKE_DISABLED,
			LIGHT_BAKE_STATIC,
			LIGHT_BAKE_DYNAMIC,
		};

	
	// Omni light 点光源
	enum LightOmniShadowMode {
		LIGHT_OMNI_SHADOW_DUAL_PARABOLOID,
		LIGHT_OMNI_SHADOW_CUBE,
	};

	virtual void light_omni_set_shadow_mode(RID p_light, LightOmniShadowMode p_mode) = 0;

	// Directional light
	// PSSM 或者说CSM
	enum LightDirectionalShadowMode {
		LIGHT_DIRECTIONAL_SHADOW_ORTHOGONAL,
		LIGHT_DIRECTIONAL_SHADOW_PARALLEL_2_SPLITS,
		LIGHT_DIRECTIONAL_SHADOW_PARALLEL_4_SPLITS,
	};

	enum LightDirectionalSkyMode {
		LIGHT_DIRECTIONAL_SKY_MODE_LIGHT_AND_SKY,
		LIGHT_DIRECTIONAL_SKY_MODE_LIGHT_ONLY,
		LIGHT_DIRECTIONAL_SKY_MODE_SKY_ONLY,
	};
	// 这边只有set
	virtual void light_directional_set_shadow_mode(RID p_light, LightDirectionalShadowMode p_mode) = 0;
	virtual void light_directional_set_blend_splits(RID p_light, bool p_enable) = 0;
	virtual void light_directional_set_sky_mode(RID p_light, LightDirectionalSkyMode p_mode) = 0;
	
	// Shadow atlas
	// 这边只有create，没有free
	virtual RID shadow_atlas_create() = 0;
	virtual void shadow_atlas_set_size(RID p_atlas, int p_size, bool p_use_16_bits = true) = 0;
	virtual void shadow_atlas_set_quadrant_subdivision(RID p_atlas, int p_quadrant, int p_subdivision) = 0;
	virtual void directional_shadow_atlas_set_size(int p_size, bool p_16_bits = true) = 0;


	enum ShadowQuality {
		SHADOW_QUALITY_HARD,
		SHADOW_QUALITY_SOFT_VERY_LOW,
		SHADOW_QUALITY_SOFT_LOW,
		SHADOW_QUALITY_SOFT_MEDIUM,
		SHADOW_QUALITY_SOFT_HIGH,
		SHADOW_QUALITY_SOFT_ULTRA,
		SHADOW_QUALITY_MAX
	};


	/* PROBE API */





public:
 	virtual void draw(bool p_swap_buffers = true, double frame_step = 0.0) = 0;
	virtual void sync() = 0;
	virtual bool has_changed() const = 0;
	virtual void init();
	virtual void finish() = 0;
	virtual void tick() = 0;
 /* STATUS INFORMATION */

	enum RenderingInfo { // 绘制指令的数量（位于viewport.h) 和 缓存的数量 (utilities.h)
		RENDERING_INFO_TOTAL_OBJECTS_IN_FRAME,
		RENDERING_INFO_TOTAL_PRIMITIVES_IN_FRAME,
		RENDERING_INFO_TOTAL_DRAW_CALLS_IN_FRAME,
		RENDERING_INFO_TEXTURE_MEM_USED,
		RENDERING_INFO_BUFFER_MEM_USED,
		RENDERING_INFO_VIDEO_MEM_USED,
		RENDERING_INFO_MAX
	};
	virtual uint64_t get_rendering_info(RenderingInfo p_info) = 0;

	struct FrameProfileArea {
		String name;
		double gpu_msec;
		double cpu_msec;
	};

		enum ShadowCastingSetting { // 是否投射阴影
		SHADOW_CASTING_SETTING_OFF,
		SHADOW_CASTING_SETTING_ON,
		SHADOW_CASTING_SETTING_DOUBLE_SIDED,
		SHADOW_CASTING_SETTING_SHADOWS_ONLY,
	};
/* ENVIRONMENT API */
	virtual RID environment_create() = 0;

	enum EnvironmentBG { // 背景类型
		ENV_BG_CLEAR_COLOR,
		ENV_BG_COLOR,
		ENV_BG_SKY,
		ENV_BG_CANVAS,
		ENV_BG_KEEP,
		ENV_BG_CAMERA_FEED,
		ENV_BG_MAX
	};

	enum EnvironmentAmbientSource {
		ENV_AMBIENT_SOURCE_BG,
		ENV_AMBIENT_SOURCE_DISABLED,
		ENV_AMBIENT_SOURCE_COLOR,
		ENV_AMBIENT_SOURCE_SKY,
	};
	enum EnvironmentReflectionSource {
		ENV_REFLECTION_SOURCE_BG,
		ENV_REFLECTION_SOURCE_DISABLED,
		ENV_REFLECTION_SOURCE_SKY,
	};

	enum EnvironmentGlowBlendMode {
		ENV_GLOW_BLEND_MODE_ADDITIVE,
		ENV_GLOW_BLEND_MODE_SCREEN,
		ENV_GLOW_BLEND_MODE_SOFTLIGHT,
		ENV_GLOW_BLEND_MODE_REPLACE,
		ENV_GLOW_BLEND_MODE_MIX,
	};

	enum EnvironmentToneMapper {
		ENV_TONE_MAPPER_LINEAR,
		ENV_TONE_MAPPER_REINHARD,
		ENV_TONE_MAPPER_FILMIC,
		ENV_TONE_MAPPER_ACES
	};

	
	enum EnvironmentSDFGIYScale {
		ENV_SDFGI_Y_SCALE_50_PERCENT,
		ENV_SDFGI_Y_SCALE_75_PERCENT,
		ENV_SDFGI_Y_SCALE_100_PERCENT,
	};


	enum EnvironmentSSAOQuality {
		ENV_SSAO_QUALITY_VERY_LOW,
		ENV_SSAO_QUALITY_LOW,
		ENV_SSAO_QUALITY_MEDIUM,
		ENV_SSAO_QUALITY_HIGH,
		ENV_SSAO_QUALITY_ULTRA,
	};

	
	enum EnvironmentSSILQuality {
		ENV_SSIL_QUALITY_VERY_LOW,
		ENV_SSIL_QUALITY_LOW,
		ENV_SSIL_QUALITY_MEDIUM,
		ENV_SSIL_QUALITY_HIGH,
		ENV_SSIL_QUALITY_ULTRA,
	};
	enum EnvironmentSSRRoughnessQuality {
		ENV_SSR_ROUGHNESS_QUALITY_DISABLED,
		ENV_SSR_ROUGHNESS_QUALITY_LOW,
		ENV_SSR_ROUGHNESS_QUALITY_MEDIUM,
		ENV_SSR_ROUGHNESS_QUALITY_HIGH,
	};
	enum SubSurfaceScatteringQuality {
		SUB_SURFACE_SCATTERING_QUALITY_DISABLED,
		SUB_SURFACE_SCATTERING_QUALITY_LOW,
		SUB_SURFACE_SCATTERING_QUALITY_MEDIUM,
		SUB_SURFACE_SCATTERING_QUALITY_HIGH,
	};
	
	virtual void environment_set_background(RID p_env, EnvironmentBG p_bg) = 0;
	virtual void environment_set_sky(RID p_env, RID p_sky) = 0;
	virtual void environment_set_sky_custom_fov(RID p_env, float p_scale) = 0;
	virtual void environment_set_sky_orientation(RID p_env, const Basis &p_orientation) = 0;
	virtual void environment_set_bg_color(RID p_env, const Color &p_color) = 0;
	virtual void environment_set_bg_energy(RID p_env, float p_multiplier, float p_exposure_value) = 0;
	virtual void environment_set_canvas_max_layer(RID p_env, int p_max_layer) = 0;
	virtual void environment_set_ambient_light(RID p_env, const Color &p_color, EnvironmentAmbientSource p_ambient = ENV_AMBIENT_SOURCE_BG, float p_energy = 1.0, float p_sky_contribution = 0.0, EnvironmentReflectionSource p_reflection_source = ENV_REFLECTION_SOURCE_BG) = 0;


	/*CAMERA EFFECT*/
	/*CAMERA EFFECT*/
	/*CAMERA EFFECT*/
	virtual RID camera_attributes_create() = 0;

	enum DOFBlurQuality {
		DOF_BLUR_QUALITY_VERY_LOW,
		DOF_BLUR_QUALITY_LOW,
		DOF_BLUR_QUALITY_MEDIUM,
		DOF_BLUR_QUALITY_HIGH,
	};

	enum DOFBokehShape {
		DOF_BOKEH_BOX,
		DOF_BOKEH_HEXAGON,
		DOF_BOKEH_CIRCLE
	};
	virtual void camera_attributes_set_dof_blur_quality(DOFBlurQuality p_quality, bool p_use_jitter) = 0;

	virtual void camera_attributes_set_dof_blur_bokeh_shape(DOFBokehShape p_shape) = 0;
	virtual void camera_attributes_set_dof_blur(RID p_camera_attributes, bool p_far_enable, float p_far_distance, float p_far_transition, bool p_near_enable, float p_near_distance, float p_near_transition, float p_amount) = 0;
	virtual void camera_attributes_set_exposure(RID p_camera_attributes, float p_multiplier, float p_exposure_normalization) = 0;
	virtual void camera_attributes_set_auto_exposure(RID p_camera_attributes, bool p_enable, float p_min_sensitivity, float p_max_sensitivity, float p_speed, float p_scale) = 0;


	virtual RID compositor_create() = 0;

	/* COMPOSITOR EFFECTS API */
	virtual RID compositor_effect_create() = 0;

	enum CompositorEffectFlags {
		COMPOSITOR_EFFECT_FLAG_ACCESS_RESOLVED_COLOR = 1,
		COMPOSITOR_EFFECT_FLAG_ACCESS_RESOLVED_DEPTH = 2,
		COMPOSITOR_EFFECT_FLAG_NEEDS_MOTION_VECTORS = 4,
		COMPOSITOR_EFFECT_FLAG_NEEDS_ROUGHNESS = 8,
		COMPOSITOR_EFFECT_FLAG_NEEDS_SEPARATE_SPECULAR = 16,
	};
	// 回调调用的时候
	enum CompositorEffectCallbackType {
		COMPOSITOR_EFFECT_CALLBACK_TYPE_PRE_OPAQUE,
		COMPOSITOR_EFFECT_CALLBACK_TYPE_POST_OPAQUE,
		COMPOSITOR_EFFECT_CALLBACK_TYPE_POST_SKY,
		COMPOSITOR_EFFECT_CALLBACK_TYPE_PRE_TRANSPARENT,
		COMPOSITOR_EFFECT_CALLBACK_TYPE_POST_TRANSPARENT,
		COMPOSITOR_EFFECT_CALLBACK_TYPE_MAX,
		COMPOSITOR_EFFECT_CALLBACK_TYPE_ANY = -1,
	};
	virtual Color get_default_clear_color() const = 0;
	virtual void set_default_clear_color(const Color &p_color)  = 0;

  virtual void free(RID p_rid) = 0;

};

}  // namespace lain
#define RS RenderingSystem
#endif  // !__RENDER_SYSTEM_H__