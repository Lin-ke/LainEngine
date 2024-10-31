#ifndef RENDERER_SCENE_CULL_H
#define RENDERER_SCENE_CULL_H
#include "core/math/dynamic_bvh.h"
#include "core/templates/bin_sorted_array.h"
#include "core/templates/list.h"
#include "core/templates/paged_array.h"
#include "core/templates/pass_func.h"
#include "function/render/rendering_system/rendering_method_api.h"
#include "renderer_geometry_instance_api.h"
#include "renderer_scene_occlusion_cull.h"
#include "renderer_scene_renderer_api.h"

namespace lain {
// RendererSceneCull: scene renderer implementation, does indexing and frustum culling using CPU
class RenderingLightCuller;
class RendererSceneCull : public RenderingMethod {
  uint64_t render_pass;

 public:
  enum { SDFGI_MAX_CASCADES = 8, SDFGI_MAX_REGIONS_PER_CASCADE = 3, MAX_INSTANCE_PAIRS = 32, MAX_UPDATE_SHADOWS = 512 };

  RendererSceneRender* scene_render = nullptr;

  static RendererSceneCull* singleton;
  struct Camera {
    enum Type { PERSPECTIVE, ORTHOGONAL, FRUSTUM };
    Type type;
    float fov;
    float znear, zfar;
    float size;
    Vector2 offset;
    uint32_t visible_layers;
    bool vaspect;
    RID env;
    RID attributes;
    RID compositor;

    Transform3D transform;

    Camera() {
      visible_layers = 0xFFFFFFFF;
      fov = 75;
      type = PERSPECTIVE;
      znear = 0.05;
      zfar = 4000;
      size = 1.0;
      offset = Vector2();
      vaspect = false;
    }
  };

  mutable RID_Owner<Camera, true> camera_owner;

  virtual RID camera_allocate();
  virtual void camera_initialize(RID p_rid);

  virtual void camera_set_perspective(RID p_camera, float p_fovy_degrees, float p_z_near, float p_z_far);
  virtual void camera_set_orthogonal(RID p_camera, float p_size, float p_z_near, float p_z_far);
  virtual void camera_set_frustum(RID p_camera, float p_size, Vector2 p_offset, float p_z_near, float p_z_far);
  virtual void camera_set_transform(RID p_camera, const Transform3D& p_transform);
  virtual void camera_set_cull_mask(RID p_camera, uint32_t p_layers);
  virtual void camera_set_environment(RID p_camera, RID p_env);
  virtual void camera_set_camera_attributes(RID p_camera, RID p_attributes);
  virtual void camera_set_compositor(RID p_camera, RID p_compositor);
  virtual void camera_set_use_vertical_aspect(RID p_camera, bool p_enable);
  virtual bool is_camera(RID p_camera) const;

  /* OCCLUDER API */
  // occluder 是通过 register注册的，因此用get_singleton方法获得单例。
  // 为什么scene_renderer是内部指针？ RenderingMethod提供一个
  virtual RID occluder_allocate();
  virtual void occluder_initialize(RID p_occluder);
  virtual void occluder_set_mesh(RID p_occluder, const PackedVector3Array& p_vertices, const PackedInt32Array& p_indices);

  /* SCENARIO API */
  // 视锥
  struct PlaneSign {
    _ALWAYS_INLINE_ PlaneSign() {}
    _ALWAYS_INLINE_ PlaneSign(const Plane& p_plane) {
      if (p_plane.normal.x > 0) {
        signs[0] = 0;
      } else {
        signs[0] = 3;
      }
      if (p_plane.normal.y > 0) {
        signs[1] = 1;
      } else {
        signs[1] = 4;
      }
      if (p_plane.normal.z > 0) {
        signs[2] = 2;
      } else {
        signs[2] = 5;
      }
    }

    uint32_t signs[3];  // 0,1,2 are positive, 3,4,5 are negative
  };
  struct Frustum {  //
    Vector<Plane> planes;
    Vector<PlaneSign> plane_signs;
    const Plane* planes_ptr;
    const PlaneSign* plane_signs_ptr;
    uint32_t plane_count;
    _ALWAYS_INLINE_ Frustum() {}
    _ALWAYS_INLINE_ Frustum(const Frustum& p_frustum) {
      planes = p_frustum.planes;
      plane_signs = p_frustum.plane_signs;

      planes_ptr = planes.ptr();
      plane_signs_ptr = plane_signs.ptr();
      plane_count = p_frustum.plane_count;
    }
    _ALWAYS_INLINE_ Frustum(const Vector<Plane>& p_planes) {
      planes = p_planes;           // ref （保持引用，避免被修改？）
      planes_ptr = planes.ptrw();  // planes_ptr和 planes不同，因为引用 > 1 。 @todo : perhaps a bug
      plane_count = planes.size();
      for (int i = 0; i < planes.size(); i++) {
        PlaneSign ps(p_planes[i]);
        plane_signs.push_back(ps);
      }

      plane_signs_ptr = plane_signs.ptr();
    }
  };

  struct InstanceBounds {  // 包围盒的两个顶点
    real_t bounds[6];
    _ALWAYS_INLINE_ InstanceBounds() {}

    _ALWAYS_INLINE_ InstanceBounds(const AABB& p_aabb) {
      bounds[0] = p_aabb.position.x;
      bounds[1] = p_aabb.position.y;
      bounds[2] = p_aabb.position.z;
      bounds[3] = p_aabb.position.x + p_aabb.size.x;
      bounds[4] = p_aabb.position.y + p_aabb.size.y;
      bounds[5] = p_aabb.position.z + p_aabb.size.z;
    }
    _ALWAYS_INLINE_ bool in_frustum(const Frustum& p_frustum) const {
      // This is not a full SAT check and the possibility of false positives exist,
      // but the tradeoff vs performance is still very good.

      for (uint32_t i = 0; i < p_frustum.plane_count; i++) {
        Vector3 min(  // 朝向的顶点
            bounds[p_frustum.plane_signs_ptr[i].signs[0]], bounds[p_frustum.plane_signs_ptr[i].signs[1]], bounds[p_frustum.plane_signs_ptr[i].signs[2]]);
        // 如果沿着法向超过朝向的顶点，说明在视锥外
        // 假阳率的原因：正好被边切割
        if (p_frustum.planes_ptr[i].distance_to(min) >= 0.0) {
          return false;
        }
      }
      return true;
    }
    // 被另一个AABB包围只需检查在三个轴的范围
    _ALWAYS_INLINE_ bool in_aabb(const AABB& p_aabb) const {
      Vector3 end = p_aabb.position + p_aabb.size;

      if (bounds[0] >= end.x) {
        return false;
      }
      if (bounds[3] <= p_aabb.position.x) {
        return false;
      }
      if (bounds[1] >= end.y) {
        return false;
      }
      if (bounds[4] <= p_aabb.position.y) {
        return false;
      }
      if (bounds[2] >= end.z) {
        return false;
      }
      if (bounds[5] <= p_aabb.position.z) {
        return false;
      }
      return true;
    }
  };
  struct Instance;
  struct InstanceData {
    enum Flags {
      FLAG_BASE_TYPE_MASK = 0xFF,
      FLAG_CAST_SHADOWS = (1 << 8),
      FLAG_CAST_SHADOWS_ONLY = (1 << 9),
      FLAG_REDRAW_IF_VISIBLE = (1 << 10),
      FLAG_GEOM_LIGHTING_DIRTY = (1 << 11),
      FLAG_GEOM_REFLECTION_DIRTY = (1 << 12),
      FLAG_GEOM_DECAL_DIRTY = (1 << 13),
      FLAG_GEOM_VOXEL_GI_DIRTY = (1 << 14),
      FLAG_LIGHTMAP_CAPTURE = (1 << 15),
      FLAG_USES_BAKED_LIGHT = (1 << 16),
      FLAG_USES_MESH_INSTANCE = (1 << 17),
      FLAG_REFLECTION_PROBE_DIRTY = (1 << 18),
      FLAG_IGNORE_OCCLUSION_CULLING = (1 << 19),
      FLAG_VISIBILITY_DEPENDENCY_NEEDS_CHECK = (3 << 20),  // 2 bits, overlaps with the other vis. dependency flags
      FLAG_VISIBILITY_DEPENDENCY_HIDDEN_CLOSE_RANGE = (1 << 20),
      FLAG_VISIBILITY_DEPENDENCY_HIDDEN = (1 << 21),
      FLAG_VISIBILITY_DEPENDENCY_FADE_CHILDREN = (1 << 22), // 设置这标志位即parent 会设置好children_fade_alpha
      FLAG_GEOM_PROJECTOR_SOFTSHADOW_DIRTY = (1 << 23),
      FLAG_IGNORE_ALL_CULLING = (1 << 24),
    };
    uint32_t flags = 0;  // 上面Flag的Bitmask
    // 这个和 Instance 的 layer_mask 保持一致
    uint32_t layer_mask = 0;  //for fast layer-mask discard
    RID base_rid;
    Instance* instance = nullptr;
    int32_t visibility_index = -1;
    union {
      // 如果light
			uint64_t instance_data_rid;
      // 如果geometry instance
      RenderGeometryInstance* instance_geometry = nullptr;
    };
    int32_t vis_parent_array_index = -1;
    // Each time occlusion culling determines an instance is visible,
		// set this to occlusion_frame plus some delay.
		// Once the timeout is reached, allow the instance to be occlusion culled.
		// This creates a delay for occlusion culling, which prevents flickering
		// when jittering the raster occlusion projection.
		uint64_t occlusion_timeout = 0;
  };

  LocalVector<Vector2> camera_jitter_array;

  struct Cull {
    struct Shadow {
      RID light_instance;
      struct Cascade {
        Frustum frustum;

        Projection projection;
        Transform3D transform;
        real_t zfar;
        real_t split;
        real_t shadow_texel_size;
        real_t bias_scale;
        real_t range_begin;
        Vector2 uv_scale;

      } cascades[RendererSceneRender::MAX_DIRECTIONAL_LIGHT_CASCADES];  //max 4 cascades
      uint32_t cascade_count;

    } shadows[RendererSceneRender::MAX_DIRECTIONAL_LIGHTS];

    uint32_t shadow_count;

    // struct SDFGI {
    //   //have arrays here because SDFGI functions expects this, plus regions can have areas
    //   AABB region_aabb[SDFGI_MAX_CASCADES * SDFGI_MAX_REGIONS_PER_CASCADE];         //max 3 regions per cascade
    //   uint32_t region_cascade[SDFGI_MAX_CASCADES * SDFGI_MAX_REGIONS_PER_CASCADE];  //max 3 regions per cascade
    //   uint32_t region_count = 0;

    //   uint32_t cascade_light_index[SDFGI_MAX_CASCADES];
    //   uint32_t cascade_light_count = 0;

    // } sdfgi;

    SpinLock lock;

    Frustum frustum;
  } cull;

  // update时填这个并在 visibility array中更新
  struct InstanceVisibilityData {
    uint64_t viewport_state = 0;  // 对哪些mask bit可见

    RS::VisibilityRangeFadeMode fade_mode = RS::VISIBILITY_RANGE_FADE_DISABLED;  // 如何fade
    Vector3 position;
    Instance* instance = nullptr;
    float range_begin = 0.0f;
    float range_end = 0.0f;
    float range_begin_margin = 0.0f;
    float range_end_margin = 0.0f;
    float children_fade_alpha = 1.0f;
    int32_t array_index = -1;  // instance_data的 index
  };
  PagedArrayPool<InstanceBounds> instance_aabb_page_pool;
  PagedArrayPool<InstanceData> instance_data_page_pool;
  PagedArrayPool<InstanceVisibilityData> instance_visibility_data_page_pool;

  struct Instance;
  // 场景
  class VisibilityArray : public BinSortedArray<InstanceVisibilityData> {
    _FORCE_INLINE_ virtual void _update_idx(InstanceVisibilityData& r_element, uint64_t p_idx) {
      r_element.instance->visibility_index = p_idx;
      if (r_element.instance->scenario && r_element.instance->array_index != -1) {
        r_element.instance->scenario->instance_data[r_element.instance->array_index].visibility_index = p_idx;
      }
    }
  };

  struct Scenario {
    enum IndexerType {
      INDEXER_GEOMETRY,  //for geometry
      INDEXER_VOLUMES,   //for everything else
      INDEXER_MAX
    };

    DynamicBVH indexers[INDEXER_MAX];

    RID self;

    List<Instance*> directional_lights;
    RID environment;
    RID fallback_environment;
    RID camera_attributes;
    RID compositor;
    RID reflection_probe_shadow_atlas;
    RID reflection_atlas;
    uint64_t used_viewport_visibility_bits;            // 已经使用的视口可见性位
    HashMap<RID, uint64_t> viewport_visibility_masks;  // viewport -> 每个视口对应的位

    SelfList<Instance>::List instances;

    LocalVector<RID> dynamic_lights;

    PagedArray<InstanceBounds> instance_aabbs;
    PagedArray<InstanceData> instance_data;
    VisibilityArray instance_visibility;

    Scenario() {
      indexers[INDEXER_GEOMETRY].set_index(INDEXER_GEOMETRY);
      indexers[INDEXER_VOLUMES].set_index(INDEXER_VOLUMES);
      used_viewport_visibility_bits = 0;
    }
  };
  struct VisibilityCullData {
    uint64_t viewport_mask;
    Scenario* scenario = nullptr;
    Vector3 camera_position;
    uint32_t cull_offset;
    uint32_t cull_count;
  };

  int indexer_update_iterations = 0;

  mutable RID_Owner<Scenario, true> scenario_owner;

  virtual RID scenario_allocate();
  virtual void scenario_initialize(RID p_rid);

  virtual void scenario_set_environment(RID p_scenario, RID p_environment);
  virtual void scenario_set_camera_attributes(RID p_scenario, RID p_attributes);
  virtual void scenario_set_fallback_environment(RID p_scenario, RID p_environment);
  virtual void scenario_set_compositor(RID p_scenario, RID p_compositor);
  virtual void scenario_set_reflection_atlas_size(RID p_scenario, int p_reflection_size, int p_reflection_count);
  virtual bool is_scenario(RID p_scenario) const;
  virtual RID scenario_get_environment(RID p_scenario);
  virtual void scenario_add_viewport_visibility_mask(RID p_scenario, RID p_viewport);
  virtual void scenario_remove_viewport_visibility_mask(RID p_scenario, RID p_viewport);

  /* INSTANCING API */
  /* INSTANCING API */
  /* INSTANCING API */
  struct InstanceBaseData;
  struct Instance {
    RS::InstanceType base_type = RS::INSTANCE_NONE;
    RID base;  // 指向实例（如灯光）的RID

    RID skeleton;
    RID material_override;
    RID material_overlay;

    RID mesh_instance;  //only used for meshes and when skeleton/blendshapes exist

    InstanceBaseData* base_data;  // 指向该实例的数据（如light，mesh等）
    Transform3D transform;
    Vector<RID> materials;  // surface 对应的override material
    uint32_t layer_mask = 1;
    AABB aabb;
    AABB transformed_aabb;
    AABB prev_transformed_aabb;
    AABB* custom_aabb = nullptr;  // 这里为什么用指针呢

    float extra_margin = 0.0;  // extra visibility margin

    bool ignore_occlusion_culling = false;
    bool ignore_all_culling = false;

    RS::ShadowCastingSetting cast_shadows;  // 此instance是否产生阴影

    struct InstanceShaderParameter {
      int32_t index = -1;
      Variant value;
      Variant default_value;
      PropertyInfo info;
    };

    RID self;  // 标识自己的RID
    DynamicBVH::ID indexer_id;
    Scenario* scenario;                // 属于的场景
    SelfList<Instance> scenario_item;  // 将这个add到场景的instances SelfList中
    bool update_aabb = false;
    bool update_dependencies = false;
    SelfList<Instance> update_item;  // 被初始化成this，用于将自己插入到 update_list中

    ObjectID object_id;

    HashMap<StringName, InstanceShaderParameter> instance_shader_uniforms;
    bool instance_allocated_shader_uniforms = false;
    int32_t instance_allocated_shader_uniforms_offset = -1;

    // visibility
    int32_t visibility_index = -1;  // 在visibility array中的index
    int32_t array_index = -1;       // 在场景instance_data 的index
    bool visible;
    Instance* visibility_parent = nullptr;
    HashSet<Instance*> visibility_dependencies;  // 依赖它的
    uint32_t visibility_dependencies_depth = 0;  // 避免出现循环
    float visibility_range_begin = 0;
    float visibility_range_end = 0;
    float visibility_range_begin_margin = 0.0f;
    float visibility_range_end_margin = 0.0f;
    RS::VisibilityRangeFadeMode visibility_range_fade_mode = RS::VISIBILITY_RANGE_FADE_DISABLED;

    // @todo DAG能否抽象一个模板类
    // 提供一个 DAG<Instance>::Node, 有root, 还有depth

    // sort
    float sorting_offset = 0.0;
    bool use_aabb_center = true;

    float transparency = 0.0;
    // dependency
    DependencyTracker dependency_tracker;
    Instance();
  };

  struct CullData {
		Cull *cull = nullptr;
		Scenario *scenario = nullptr;
		RID shadow_atlas;
		Transform3D cam_transform;
		uint32_t visible_layers;
		Instance *render_reflection_probe = nullptr;
		const RendererSceneOcclusionCull::HZBuffer *occlusion_buffer;
		const Projection *camera_matrix;
		uint64_t visibility_viewport_mask;
	};
  struct InstanceBaseData {
    virtual ~InstanceBaseData() {}
  };
  struct InstanceGeometryData : public InstanceBaseData {
    RenderGeometryInstance* geometry_instance = nullptr;
    bool can_cast_shadows = false;
    HashSet<Instance*> lights;  // 照射该instance的灯光
    bool material_is_animated = false;
  };
  struct InstanceReflectionProbeData : public InstanceBaseData {};
  struct InstanceLightData : public InstanceBaseData {

    RID instance;
    // Instead of a single dirty flag, we maintain a count
    // so that we can detect lights that are being made dirty
    // each frame, and switch on tighter caster culling.
    int32_t shadow_dirty_count;
    uint32_t light_update_frame_id;
    bool light_intersects_multiple_cameras;
    uint32_t light_intersects_multiple_cameras_timeout_frame_id;
    RS::LightBakeMode bake_mode;
		uint64_t last_version;
    // 通过这种方式保存了对链表整体的引用
    List<Instance*>::Element* D;  // 存到场景的directional_lights中的返回值
		bool is_shadow_dirty() const { return shadow_dirty_count != 0; }
    
    void make_shadow_dirty() { shadow_dirty_count = light_intersects_multiple_cameras ? 1 : 2; }
    // 一个light在p_frame_id帧中是否被多个camera看到（调用该函数）
    // 多线程？
    void detect_light_intersects_multiple_cameras(uint32_t p_frame_id) {
			// We need to detect the case where shadow updates are occurring
			// more than once per frame. In this case, we need to turn off
			// tighter caster culling, so situation reverts to one full shadow update
			// per frame (light_intersects_multiple_cameras is set).
			if (p_frame_id == light_update_frame_id) {
				light_intersects_multiple_cameras = true;
				light_intersects_multiple_cameras_timeout_frame_id = p_frame_id + 60;
			} else {
				// When shadow_volume_intersects_multiple_cameras is set, we
				// want to detect the situation this is no longer the case, via a timeout.
				// The system can go back to tighter caster culling in this situation.
				// Having a long-ish timeout prevents rapid cycling.
				if (light_intersects_multiple_cameras && (p_frame_id >= light_intersects_multiple_cameras_timeout_frame_id)) {
					light_intersects_multiple_cameras = false;
					light_intersects_multiple_cameras_timeout_frame_id = UINT32_MAX;
				}
			}
			light_update_frame_id = p_frame_id;
		}
    void decrement_shadow_dirty() {
			shadow_dirty_count--;
			DEV_ASSERT(shadow_dirty_count >= 0);
		}
    InstanceLightData() {
			bake_mode = RS::LIGHT_BAKE_DISABLED;
			D = nullptr;
			last_version = 0;
			// baked_light = nullptr;

			shadow_dirty_count = 1;
			light_update_frame_id = UINT32_MAX;
			light_intersects_multiple_cameras_timeout_frame_id = UINT32_MAX;
			light_intersects_multiple_cameras = false;
		}
  };
  struct InstanceDecalData : public InstanceBaseData {};

  struct InstanceVoxelGIData : public InstanceBaseData {
    bool invalid;
    uint32_t base_version;
    SelfList<InstanceVoxelGIData> update_element;
    InstanceVoxelGIData() : update_element(this) {
      invalid = true;
      base_version = 0;
    }
  };
  SelfList<InstanceVoxelGIData>::List voxel_gi_update_list;

  struct InstanceLightmapData : public InstanceBaseData {};
  struct InstanceOccluderData : public InstanceBaseData {};
  struct InstanceVisibilityNotifierData : public InstanceBaseData {};
  struct InstanceFogVolumeData : public InstanceBaseData {};
  struct InstanceParticlesCollisionData : public InstanceBaseData {
    RID instance;
  };
  	PagedArray<Instance *> instance_cull_result;
	PagedArray<Instance *> instance_shadow_cull_result;
  struct InstanceCullResult {  // 这里全用RID，感觉PageArray就很没意义。。
    PagedArray<RenderGeometryInstance*> geometry_instances;
    PagedArray<Instance*> lights;
    PagedArray<RID> light_instances;
    PagedArray<RID> lightmaps;
    PagedArray<RID> reflections;
    PagedArray<RID> decals;
    PagedArray<RID> voxel_gi_instances;
    PagedArray<RID> mesh_instances;
    PagedArray<RID> fog_volumes;
    struct DirectionalShadow {
      PagedArray<RenderGeometryInstance*> cascade_geometry_instances[RendererSceneRender::MAX_DIRECTIONAL_LIGHT_CASCADES];  // 为啥这些都是Geometry
    } directional_shadows[RendererSceneRender::MAX_DIRECTIONAL_LIGHTS];
    PagedArray<RenderGeometryInstance*> sdfgi_region_geometry_instances[SDFGI_MAX_CASCADES * SDFGI_MAX_REGIONS_PER_CASCADE];  // 为啥不在scene renderer里
    PagedArray<RID> sdfgi_cascade_lights[SDFGI_MAX_CASCADES];

    void clear();
    void reset();
    void append_from(InstanceCullResult& p_cull_result);
    void init(PagedArrayPool<RID>* p_rid_pool, PagedArrayPool<RenderGeometryInstance*>* p_geometry_instance_pool, PagedArrayPool<Instance*>* p_instance_pool);  // setpool
    // page array的实现允许用一个池
  };

  InstanceCullResult scene_cull_result;
  LocalVector<InstanceCullResult> scene_cull_result_threads;
  RendererSceneRender::RenderShadowData render_shadow_data[MAX_UPDATE_SHADOWS]; // 这里填写所有的shadow数据（在render_scene里）
  uint32_t max_shadows_used = 0;
  uint32_t thread_cull_threshold = 200;  // 多线程cull

  // RendererSceneRender::RenderSDFGIData render_sdfgi_data[SDFGI_MAX_CASCADES * SDFGI_MAX_REGIONS_PER_CASCADE];
  // RendererSceneRender::RenderSDFGIUpdateData sdfgi_update_data;

  RID_Owner<Instance, true> instance_owner;
  virtual RID instance_allocate();
  virtual void instance_initialize(RID p_rid);
  virtual void instance_set_base(RID p_instance, RID p_base);
  virtual void instance_set_scenario(RID p_instance, RID p_scenario);
  virtual void instance_set_layer_mask(RID p_instance, uint32_t p_mask);
  virtual void instance_set_pivot_data(RID p_instance, float p_sorting_offset, bool p_use_aabb_center);
  virtual void instance_set_transform(RID p_instance, const Transform3D& p_transform);
  virtual void instance_attach_object_instance_id(RID p_instance, ObjectID p_id);
  virtual void instance_set_blend_shape_weight(RID p_instance, int p_shape, float p_weight);
  virtual void instance_set_surface_override_material(RID p_instance, int p_surface, RID p_material);
  virtual void instance_set_visible(RID p_instance, bool p_visible);
  virtual void instance_geometry_set_transparency(RID p_instance, float p_transparency);
  virtual void instance_set_custom_aabb(RID p_instance, AABB p_aabb) override;
  virtual void instance_set_extra_visibility_margin(RID p_instance, real_t p_margin) override;
  virtual void instance_set_ignore_culling(RID p_instance, bool p_enabled) override;
  virtual void instance_attach_skeleton(RID p_instance, RID p_skeleton) override;
  virtual void instance_set_visibility_parent(RID p_instance, RID p_parent_instance) override;

  // 直接memnew()
  RendererSceneOcclusionCull* dummy_occlusion_culling = nullptr;

#ifdef PASSBASE
#undef PASSBASE
#endif

#define PASSBASE scene_render

  PASS0R(RID, environment_allocate)
  PASS1(environment_initialize, RID)
  PASS1RC(bool, is_environment, RID)
  PASS1RC(RS::EnvironmentBG, environment_get_background, RID)
  PASS1RC(int, environment_get_canvas_max_layer, RID)
  PASS0R(Ref<RenderSceneBuffers>, render_buffers_create)
#undef PASSBASE

  virtual void render_camera(const Ref<RenderSceneBuffers>& p_render_buffers, RID p_camera, RID p_scenario, RID p_viewport, Size2 p_viewport_size,
                             uint32_t p_jitter_phase_count, float p_screen_mesh_lod_threshold, RID p_shadow_atlas,
                             RenderingMethod::RenderInfo* r_render_info = nullptr) override;
  virtual void render_empty_scene(const Ref<RenderSceneBuffers>& p_render_buffers, RID p_scenario, RID p_shadow_atlas) override;
  virtual void render_probes() override;

  // update
  virtual void update();
  virtual void update_visibility_notifiers();
  void update_dirty_instances();
  void render_particle_colliders() {}

  void set_scene_render(RendererSceneRender* p_scene_render) { scene_render = p_scene_render; }
  RenderingLightCuller* light_culler = nullptr;

 private:
  SelfList<Instance>::List _instance_update_list;
  void _instance_update_mesh_instance(Instance* p_instance);
  void _instance_queue_update(Instance* p_instance, bool p_update_aabb = false, bool p_update_dependencies = false);
  void _update_dirty_instance(Instance* p_instance);
  void _update_instance_aabb(Instance*);
  void _update_instance_shader_uniforms_from_material(HashMap<StringName, Instance::InstanceShaderParameter>& isparams,
                                                      const HashMap<StringName, Instance::InstanceShaderParameter>& existing_isparams, RID p_material);
  void _unpair_instance(Instance* p_instance);
  bool _update_instance_visibility_depth(Instance* p_instance);
  void _update_instance_visibility_dependencies(Instance* p_instance);
  RID _render_get_environment(RID camera, RID p_scenario);
  RID _render_get_compositor(RID camera, RID p_scenario);

  void _render_scene(const RendererSceneRender::CameraData* p_camera_data, const Ref<RenderSceneBuffers>& p_render_buffers, RID p_environment,
                                        RID p_force_camera_attributes, RID p_compositor, uint32_t p_visible_layers, RID p_scenario, RID p_viewport, RID p_shadow_atlas,
                                        RID p_reflection_probe, int p_reflection_probe_pass, float p_screen_mesh_lod_threshold, bool p_using_shadows,
                                        RenderingMethod::RenderInfo* r_render_info);
  void _visibility_cull_threaded(uint32_t p_thread, VisibilityCullData* cull_data);
  void _visibility_cull(const VisibilityCullData& cull_data, uint64_t p_from, uint64_t p_to);
  template <bool p_fade_check>
  _FORCE_INLINE_ int _visibility_range_check(InstanceVisibilityData& r_vis_data, const Vector3& p_camera_pos, uint64_t p_viewport_mask);
  void _light_instance_setup_directional_shadow(int p_shadow_index, Instance* p_instance, const Transform3D p_cam_transform, const Projection& p_cam_projection,
                                                bool p_cam_orthogonal, bool p_cam_vaspect);
  void _scene_cull_threaded(uint32_t p_thread, CullData *cull_data);
	void _scene_cull(CullData &cull_data, InstanceCullResult &cull_result, uint64_t p_from, uint64_t p_to);
  bool _visibility_parent_check(const CullData &p_cull_data, const InstanceData &p_instance_data);
	bool _light_instance_update_shadow(Instance *p_instance, const Transform3D p_cam_transform, const Projection &p_cam_projection, bool p_cam_orthogonal, bool p_cam_vaspect, RID p_shadow_atlas, Scenario *p_scenario, float p_scren_mesh_lod_threshold, uint32_t p_visible_layers = 0xFFFFFF);

	L_INLINE bool _is_colinear_tri(const Vector3 &p_a, const Vector3 &p_b, const Vector3 &p_c) const {
		// Lengths of sides a, b and c.
		float la = (p_b - p_a).length();
		float lb = (p_c - p_b).length();
		float lc = (p_c - p_a).length();

		// Get longest side into lc.
		if (lb < la) {
			SWAP(la, lb);
		}
		if (lc < lb) {
			SWAP(lb, lc);
		}

		// Prevent divide by zero.
		if (lc > 0.001f) {
			// If the summed length of the smaller two
			// sides is close to the length of the longest side,
			// the points are colinear, and the triangle is near degenerate.
			float ld = ((la + lb) - lc) / lc;

			// ld will be close to zero for colinear tris.
			return ld < 0.001f;
		}

		// Don't create planes from tiny triangles,
		// they won't be accurate.
		return true;
	}

};
}  // namespace lain
#endif