#pragma once
#ifndef __RENDERING_SYSTEM_H__
#define __RENDERING_SYSTEM_H__
#include "base.h"
#include "core/object/object.h"
#include "core/templates/hash_set.h"
#include "function/display/window_system.h"
#include "function/render/common/rendering_device.h"
namespace lain {

class RenderingSystem : public Object {
  LCLASS(RenderingSystem, Object);

 public:
  RenderingSystem() { p_singleton = this; };
  virtual ~RenderingSystem() { p_singleton = nullptr; }
  L_INLINE static RenderingSystem* get_singleton() { return p_singleton; }
  bool is_render_loop_enabled() const { return render_loop_enabled; }
  void set_render_loop_enabled(bool p_enabled) { render_loop_enabled = p_enabled; }

 private:
  static RenderingSystem* p_singleton;
  bool render_loop_enabled = true;

 public:
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

 public:
 /* STATUS INFORMATION */

	enum RenderingInfo {
		RENDERING_INFO_TOTAL_OBJECTS_IN_FRAME,
		RENDERING_INFO_TOTAL_PRIMITIVES_IN_FRAME,
		RENDERING_INFO_TOTAL_DRAW_CALLS_IN_FRAME,
		RENDERING_INFO_TEXTURE_MEM_USED,
		RENDERING_INFO_BUFFER_MEM_USED,
		RENDERING_INFO_VIDEO_MEM_USED,
		RENDERING_INFO_MAX
	};
	
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

  struct SurfaceData {
    PrimitiveType primitive = PRIMITIVE_MAX;

    uint64_t format = ARRAY_FLAG_FORMAT_CURRENT_VERSION;
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
};

}  // namespace lain
#define RS RenderingSystem
#endif  // !__RENDER_SYSTEM_H__