#ifndef LIGHT_STORAGE_RD_H
#define LIGHT_STORAGE_RD_H
#include "function/render/rendering_system/light_storage_api.h"
#include "function/render/rendering_system/utilities.h"
namespace lain::RendererRD {
class LightStorage : public RendererLightStorage {
  static LightStorage* singleton;

 public:
  LightStorage();
  virtual ~LightStorage();
  static LightStorage* get_singleton() { return singleton; }
  bool owns_reflection_probe(RID p_rid);
  bool owns_light(RID p_rid);
  bool owns_lightmap(RID p_rid);

 private:
  /* LIGHT */
  struct Light {
    RS::LightType type;
    float param[RS::LIGHT_PARAM_MAX];  // 光照参数
    Color color = Color(1, 1, 1, 1);
    RID projector;
    bool shadow = false;
    bool negative = false; // subtract light
    bool reverse_cull = false; // 翻转网格的背面剔除
    RS::LightBakeMode bake_mode = RS::LIGHT_BAKE_DYNAMIC;
    uint32_t max_sdfgi_cascade = 2;  // 这个被设置为可以调整的
    uint32_t cull_mask = 0xFFFFFFFF; // 光只影响这个mask上的
    bool distance_fade = false;
    real_t distance_fade_begin = 40.0;
    real_t distance_fade_shadow = 50.0;
    real_t distance_fade_length = 10.0;
    RS::LightOmniShadowMode omni_shadow_mode = RS::LIGHT_OMNI_SHADOW_DUAL_PARABOLOID;
    RS::LightDirectionalShadowMode directional_shadow_mode = RS::LIGHT_DIRECTIONAL_SHADOW_ORTHOGONAL;
    bool directional_blend_splits = false;
    RS::LightDirectionalSkyMode directional_sky_mode = RS::LIGHT_DIRECTIONAL_SKY_MODE_LIGHT_AND_SKY;
    uint64_t version = 0;

    Dependency dependency; // 持有 Denpendency 就可以通过callback通知所有的tracker
  };
  mutable RID_Owner<Light, true> light_owner;

  /* LIGHT INSTANCE */

  struct LightInstance {
    struct ShadowTransform {
      Projection camera;
      Transform3D transform;
      float farplane = 0.0;
      float split = 0.0;
      float bias_scale = 0.0;
      float shadow_texel_size = 0.0;
      float range_begin = 0.0;
      Rect2 atlas_rect;
      Vector2 uv_scale;
    };

    RS::LightType light_type = RS::LIGHT_DIRECTIONAL;

    ShadowTransform shadow_transform[6];

    AABB aabb; // instance 的AABB和light的AABB有什么区别？
    RID self; // 
    RID light; // sturct Light's RID
    Transform3D transform;

    Vector3 light_vector;
    Vector3 spot_vector;
    float linear_att = 0.0; // ?

    uint64_t shadow_pass = 0; // pass记录
    uint64_t last_scene_pass = 0;
    uint64_t last_scene_shadow_pass = 0;
    uint64_t last_pass = 0;
    uint32_t cull_mask = 0;
    uint32_t light_directional_index = 0;

    Rect2 directional_rect;

    HashSet<RID> shadow_atlases;  //shadow atlases where this light is registered

    // ForwardID forward_id = -1;

    LightInstance() {}
  };

  mutable RID_Owner<LightInstance> light_instance_owner;

  /* OMNI/SPOT LIGHT DATA */

  struct LightData {
    float position[3];
    float inv_radius;
    float direction[3];  // in omni, x and y are used for dual paraboloid offset
    float size;

    float color[3];
    float attenuation;

    float inv_spot_attenuation;
    float cos_spot_angle;
    float specular_amount;
    float shadow_opacity;

    float atlas_rect[4];  // in omni, used for atlas uv, in spot, used for projector uv
    float shadow_matrix[16];
    float shadow_bias;
    float shadow_normal_bias;
    float transmittance_bias;
    float soft_shadow_size;
    float soft_shadow_scale;
    uint32_t mask;
    float volumetric_fog_energy;
    uint32_t bake_mode;
    float projector_rect[4];
  };

  struct LightInstanceDepthSort {
    float depth;
    LightInstance* light_instance;
    Light* light;
    bool operator<(const LightInstanceDepthSort& p_sort) const { return depth < p_sort.depth; }
  };

  uint32_t max_lights;
  uint32_t omni_light_count = 0;
  uint32_t spot_light_count = 0;
  LightData* omni_lights = nullptr;
  LightData* spot_lights = nullptr;
  LightInstanceDepthSort* omni_light_sort = nullptr;
  LightInstanceDepthSort* spot_light_sort = nullptr;
  RID omni_light_buffer;
  RID spot_light_buffer;

  /* DIRECTIONAL LIGHT DATA */

  struct DirectionalLightData {
    float direction[3];
    float energy;
    float color[3];
    float size;
    float specular;
    uint32_t mask;
    float softshadow_angle;
    float soft_shadow_scale;
    uint32_t blend_splits;
    float shadow_opacity;
    float fade_from;
    float fade_to;
    uint32_t pad[2];
    uint32_t bake_mode;
    float volumetric_fog_energy;
    float shadow_bias[4];
    float shadow_normal_bias[4];
    float shadow_transmittance_bias[4];
    float shadow_z_range[4];
    float shadow_range_begin[4];
    float shadow_split_offsets[4];
    float shadow_matrices[4][16];
    float uv_scale1[2];
    float uv_scale2[2];
    float uv_scale3[2];
    float uv_scale4[2];
  };

  uint32_t max_directional_lights;
  DirectionalLightData* directional_lights = nullptr;
  RID directional_light_buffer;

  /* DIRECTIONAL SHADOW */

  struct DirectionalShadow {
    RID depth;
    RID fb;  //when rendering direct //@typo

    int light_count = 0;
    int size = 0;
    bool use_16_bits = true;
    int current_light = 0;
  } directional_shadow;

  struct ShadowCubemap {
    RID cubemap; // 指向对应的那个纹理
    RID side_fb[6];
  };
  HashMap<int, ShadowCubemap> shadow_cubemaps; // size 到 cubemap的映射
 
  private:

 public:
  // APIS

  virtual RID directional_light_allocate() override;
  virtual void directional_light_initialize(RID p_rid) override;

  virtual RID omni_light_allocate() override;
  virtual void omni_light_initialize(RID p_rid) override;

  virtual RID spot_light_allocate() override;
  virtual void spot_light_initialize(RID p_rid) override;

  virtual void light_free(RID p_rid) override;

  virtual void light_set_color(RID p_light, const Color& p_color) override;
  virtual void light_set_param(RID p_light, RS::LightParam p_param, float p_value) override;
  virtual void light_set_shadow(RID p_light, bool p_enabled) override;
  virtual void light_set_projector(RID p_light, RID p_texture) override;
  virtual void light_set_negative(RID p_light, bool p_enable) override;
  virtual void light_set_cull_mask(RID p_light, uint32_t p_mask) override;
  virtual void light_set_distance_fade(RID p_light, bool p_enabled, float p_begin, float p_shadow, float p_length) override;
  virtual void light_set_reverse_cull_face_mode(RID p_light, bool p_enabled) override;
  virtual void light_set_bake_mode(RID p_light, RS::LightBakeMode p_bake_mode) override;
  virtual void light_set_max_sdfgi_cascade(RID p_light, uint32_t p_cascade) override;

  virtual void light_omni_set_shadow_mode(RID p_light, RS::LightOmniShadowMode p_mode) override;

  virtual void light_directional_set_shadow_mode(RID p_light, RS::LightDirectionalShadowMode p_mode) override;
  virtual void light_directional_set_blend_splits(RID p_light, bool p_enable) override;
  virtual void light_directional_set_sky_mode(RID p_light, RS::LightDirectionalSkyMode p_mode) override;
	Dependency *light_get_dependency(RID p_light) const;

  virtual bool light_directional_get_blend_splits(RID p_light) const override;
  virtual RS::LightDirectionalSkyMode light_directional_get_sky_mode(RID p_light) const override;
  virtual RS::LightDirectionalShadowMode light_directional_get_shadow_mode(RID p_light) override;
  virtual RS::LightOmniShadowMode light_omni_get_shadow_mode(RID p_light) override;
  // 这些应该都拿宏来写
  virtual bool light_has_shadow(RID p_light) const override;

  virtual bool light_has_projector(RID p_light) const override;

  virtual RS::LightType light_get_type(RID p_light) const override;
  virtual AABB light_get_aabb(RID p_light) const override;
  virtual float light_get_param(RID p_light, RS::LightParam p_param) override;
  virtual Color light_get_color(RID p_light) override;
  virtual bool light_get_reverse_cull_face_mode(RID p_light) const override;
  virtual RS::LightBakeMode light_get_bake_mode(RID p_light) override;
  virtual uint32_t light_get_max_sdfgi_cascade(RID p_light) override;
  virtual uint64_t light_get_version(RID p_light) const override;
  virtual uint32_t light_get_cull_mask(RID p_light) const override;

  /* LIGHT INSTANCE API */

  virtual RID light_instance_create(RID p_light) override;
  virtual void light_instance_free(RID p_light_instance) override;
  virtual void light_instance_set_transform(RID p_light_instance, const Transform3D& p_transform) override;
  virtual void light_instance_set_aabb(RID p_light_instance, const AABB& p_aabb) override;
  virtual void light_instance_set_shadow_transform(RID p_light_instance, const Projection& p_projection, const Transform3D& p_transform, float p_far, float p_split,
                                                   int p_pass, float p_shadow_texel_size, float p_bias_scale = 1.0, float p_range_begin = 0,
                                                   const Vector2& p_uv_scale = Vector2()) override;
  virtual void light_instance_mark_visible(RID p_light_instance) override;
  virtual bool light_instance_is_shadow_visible_at_position(RID p_light, const Vector3& p_position) const override;

  // 一些get——set方法
  _FORCE_INLINE_ RID light_instance_get_base_light(RID p_light_instance) {
		LightInstance *li = light_instance_owner.get_or_null(p_light_instance);
		return li->light;
	}

	_FORCE_INLINE_ Transform3D light_instance_get_base_transform(RID p_light_instance) {
		LightInstance *li = light_instance_owner.get_or_null(p_light_instance);
		return li->transform;
	}

	_FORCE_INLINE_ AABB light_instance_get_base_aabb(RID p_light_instance) {
		LightInstance *li = light_instance_owner.get_or_null(p_light_instance);
		return li->aabb;
	}

	_FORCE_INLINE_ void light_instance_set_cull_mask(RID p_light_instance, uint32_t p_cull_mask) {
		LightInstance *li = light_instance_owner.get_or_null(p_light_instance);
		li->cull_mask = p_cull_mask;
	}

	_FORCE_INLINE_ uint32_t light_instance_get_cull_mask(RID p_light_instance) {
		LightInstance *li = light_instance_owner.get_or_null(p_light_instance);
		return li->cull_mask;
	}

   struct ShadowAtlas {
		struct Quadrant {
			uint32_t subdivision = 0; // 细分？

			struct Shadow { // 并没有实际的存储呀
				RID owner; // LightInstance 
				uint64_t version = 0;
				uint64_t fog_version = 0; // used for fog
				uint64_t alloc_tick = 0; // 分配时的tick

				Shadow() {}
			};

			Vector<Shadow> shadows;

			Quadrant() {}
		} quadrants[4]; // 四个象限，每个象限有许多Shadow

		int size_order[4] = { 0, 1, 2, 3 }; // 排序四个象限的subdivision
		uint32_t smallest_subdiv = 0; // 最小divison，用于快速分配

		int size = 0; // 整个长度，考虑到是正方形，单个quad_size = size >> 1
		bool use_16_bits = true;

		RID depth; // texture_create (TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		RID fb; //for copying
		HashMap<RID, uint32_t> shadow_owners; // lights -> 对应的阴影
    // uint32_T : 象限 + index的阴影
	};
  const	uint64_t shadow_atlas_realloc_tolerance_msec = 500;

  RID_Alloc<ShadowAtlas> shadow_atlas_owner;
  enum ShadowAtlastQuadrant {
		QUADRANT_SHIFT = 27,
		OMNI_LIGHT_FLAG = 1 << 26,
		SHADOW_INDEX_MASK = OMNI_LIGHT_FLAG - 1,
		SHADOW_INVALID = 0xFFFFFFFF
	};

  virtual RID shadow_atlas_create() override;
	virtual void shadow_atlas_free(RID p_atlas) override;

	virtual void shadow_atlas_set_size(RID p_atlas, int p_size, bool p_16_bits = true) override;
	virtual void shadow_atlas_set_quadrant_subdivision(RID p_atlas, int p_quadrant, int p_subdivision) override;
	virtual bool shadow_atlas_update_light(RID p_atlas, RID p_light_instance, float p_coverage, uint64_t p_light_version) override;
	virtual void shadow_atlas_update(RID p_atlas) override;
 	virtual void directional_shadow_atlas_set_size(int p_size, bool p_16_bits = true) override;
	virtual int get_directional_light_shadow_size(RID p_light_instance) override;
	virtual void set_directional_shadow_count(int p_count) override;

  	/* SHADOW CUBEMAPS */

	RID get_cubemap(int p_size);
  // size 为 p_size 的第p_pass个
	RID get_cubemap_fb(int p_size, int p_pass);
private:
	void _light_initialize(RID p_rid, RS::LightType p_type);
  void _shadow_atlas_invalidate_shadow(ShadowAtlas::Quadrant::Shadow *p_shadow, RID p_atlas, ShadowAtlas *p_shadow_atlas, uint32_t p_quadrant, uint32_t p_shadow_idx);
	// 从 count 往前 直到 current 中寻找 可以被替代的 shadow
  bool _shadow_atlas_find_shadow(ShadowAtlas *shadow_atlas, int *p_in_quadrants, int p_quadrant_count, int p_current_subdiv, uint64_t p_tick, int &r_quadrant, int &r_shadow);
	// shadow pair
  bool _shadow_atlas_find_omni_shadows(ShadowAtlas *shadow_atlas, int *p_in_quadrants, int p_quadrant_count, int p_current_subdiv, uint64_t p_tick, int &r_quadrant, int &r_shadow);
  // 新建texture
  void _update_shadow_atlas(ShadowAtlas* p_shadow_atlas);
  // 
  ShadowCubemap *_get_shadow_cubemap(int p_size);

};
}  // namespace lain::RendererRD
#endif