#ifndef RENDERER_SCENE_CULL_H
#define RENDERER_SCENE_CULL_H
#include "core/math/dynamic_bvh.h"
#include "core/templates/bin_sorted_array.h"
#include "core/templates/paged_array.h"
#include "function/render/rendering_system/rendering_method_api.h"
#include "renderer_geometry_instance_api.h"
#include "renderer_scene_renderer_api.h"
namespace lain {
// RendererSceneCull: scene renderer implementation, does indexing and frustum culling using CPU
class RendererSceneCull : public RenderingMethod {
 public:

 enum {
		SDFGI_MAX_CASCADES = 8,
		SDFGI_MAX_REGIONS_PER_CASCADE = 3,
		MAX_INSTANCE_PAIRS = 32,
		MAX_UPDATE_SHADOWS = 512
	};
	RendererSceneRender *scene_render = nullptr;
 
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

  virtual RID occluder_allocate();
  virtual void occluder_initialize(RID p_occluder);
  virtual void occluder_set_mesh(RID p_occluder, const PackedVector3Array& p_vertices, const PackedInt32Array& p_indices);

  /* SCENARIO API */
// 视锥
	struct PlaneSign {
		_ALWAYS_INLINE_ PlaneSign() {}
		_ALWAYS_INLINE_ PlaneSign(const Plane &p_plane) {
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

		uint32_t signs[3]; // 0,1,2 are positive, 3,4,5 are negative
	};
	struct Frustum { // 
		Vector<Plane> planes;
		Vector<PlaneSign> plane_signs;
		const Plane *planes_ptr;
		const PlaneSign *plane_signs_ptr;
		uint32_t plane_count;
		_ALWAYS_INLINE_ Frustum() {}
		_ALWAYS_INLINE_ Frustum(const Frustum &p_frustum){
			planes = p_frustum.planes;
			plane_signs = p_frustum.plane_signs;

			planes_ptr = planes.ptr();
			plane_signs_ptr = plane_signs.ptr();
			plane_count = p_frustum.plane_count;
		}
		_ALWAYS_INLINE_ Frustum(const Vector<Plane> &p_planes) {
			planes = p_planes; // ref （保持引用，避免被修改？）
			planes_ptr = planes.ptrw(); // planes_ptr和 planes不同，因为引用 > 1 。 @todo : perhaps a bug
			plane_count = planes.size();
			for (int i = 0; i < planes.size(); i++) {
				PlaneSign ps(p_planes[i]);
				plane_signs.push_back(ps);
			}

			plane_signs_ptr = plane_signs.ptr();
		}
	};

  struct InstanceBounds { // 包围盒的两个顶点
		real_t bounds[6]; 
		_ALWAYS_INLINE_ InstanceBounds() {}

		_ALWAYS_INLINE_ InstanceBounds(const AABB &p_aabb) {
			bounds[0] = p_aabb.position.x;
			bounds[1] = p_aabb.position.y;
			bounds[2] = p_aabb.position.z;
			bounds[3] = p_aabb.position.x + p_aabb.size.x;
			bounds[4] = p_aabb.position.y + p_aabb.size.y;
			bounds[5] = p_aabb.position.z + p_aabb.size.z;
		}
		_ALWAYS_INLINE_ bool in_frustum(const Frustum &p_frustum) const {
			// This is not a full SAT check and the possibility of false positives exist,
			// but the tradeoff vs performance is still very good.

			for (uint32_t i = 0; i < p_frustum.plane_count; i++) {
				Vector3 min( // 朝向的顶点
						bounds[p_frustum.plane_signs_ptr[i].signs[0]],
						bounds[p_frustum.plane_signs_ptr[i].signs[1]],
						bounds[p_frustum.plane_signs_ptr[i].signs[2]]);
				 // 如果沿着法向超过朝向的顶点，说明在视锥外
				 // 假阳率的原因：正好被边切割
				if (p_frustum.planes_ptr[i].distance_to(min) >= 0.0) {
					return false;
				}
			}
			return true;
		} 
		// 被另一个AABB包围只需检查在三个轴的范围
		_ALWAYS_INLINE_ bool in_aabb(const AABB &p_aabb) const {
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
  struct InstanceData{
		uint32_t flags = 0;
		uint32_t layer_mask = 0; //for fast layer-mask discard
		RID base_rid;
		Instance *instance = nullptr;
	};
  struct InstanceVisibilityData {
    uint64_t viewport_state = 0;
    int32_t array_index = -1;
    RS::VisibilityRangeFadeMode fade_mode = RS::VISIBILITY_RANGE_FADE_DISABLED; // 如何fade
    Vector3 position;
    Instance* instance = nullptr;
    float range_begin = 0.0f;
    float range_end = 0.0f;
    float range_begin_margin = 0.0f;
    float range_end_margin = 0.0f;
    float children_fade_alpha = 1.0f;
  };
	PagedArrayPool<InstanceBounds> instance_aabb_page_pool;
	PagedArrayPool<InstanceData> instance_data_page_pool;
	PagedArrayPool<InstanceVisibilityData> instance_visibility_data_page_pool;
  // class VisibilityArray : public BinSortedArray<InstanceVisibilityData> {
  //   _FORCE_INLINE_ virtual void _update_idx(InstanceVisibilityData& r_element, uint64_t p_idx) {
  //     r_element.instance->visibility_index = p_idx;
  //     if (r_element.instance->scenario && r_element.instance->array_index != -1) {
  //       r_element.instance->scenario->instance_data[r_element.instance->array_index].visibility_index = p_idx;
  //     }
  //   }
  // };
	struct Instance;
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
    uint64_t used_viewport_visibility_bits;
    HashMap<RID, uint64_t> viewport_visibility_masks;

    SelfList<Instance>::List instances;

    LocalVector<RID> dynamic_lights;

    PagedArray<InstanceBounds> instance_aabbs;
    PagedArray<InstanceData> instance_data;
    // VisibilityArray instance_visibility;

    Scenario() {
      indexers[INDEXER_GEOMETRY].set_index(INDEXER_GEOMETRY);
      indexers[INDEXER_VOLUMES].set_index(INDEXER_VOLUMES);
      used_viewport_visibility_bits = 0;
    }
  };

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
	struct Instance{
		RS::InstanceType base_type = RS::INSTANCE_NONE;
		RID base; // 指向实例（如灯光）的RID
		InstanceBaseData* base_data; // 指向实例的数据
		Transform3D transform;
		Vector<RID> materials;
		uint32_t layer_mask = 1;
		AABB aabb;
		RID self; // 标识自己的RID
		Scenario* scenario; // 属于的场景
		bool update_aabb = false;
		bool update_dependencies = false;
		SelfList<Instance> update_item; // 被初始化成this，用于将自己插入到 update_list中
		

		// visibility
		// dependency
		DependencyTracker dependency_tracker;
		Instance();
	};
  struct InstanceBaseData {
		virtual ~InstanceBaseData() {}
	};
	struct InstanceGeometryData : public InstanceBaseData{
		RenderGeometryInstance *geometry_instance = nullptr; 
	}; 
	struct InstanceReflectionProbeData : public InstanceBaseData{};
	struct InstanceLightData : public InstanceBaseData{};
	struct InstanceDecalData : public InstanceBaseData{};
	struct InstanceVoxelGIData : public InstanceBaseData{};
	struct InstanceLightmapData : public InstanceBaseData{};
	struct InstanceOccluderData : public InstanceBaseData{};
	struct InstanceVisibilityNotifierData : public InstanceBaseData{};
	struct InstanceFogVolumeData : public InstanceBaseData{};
	struct InstanceParticlesCollisionData : public InstanceBaseData {
			RID instance;
		};
	struct InstanceCullResult { // 这里全用RID，感觉PageArray就很没意义。。
		PagedArray<RenderGeometryInstance *> geometry_instances;
		PagedArray<Instance *> lights;
		PagedArray<RID> light_instances;
		PagedArray<RID> lightmaps;
		PagedArray<RID> reflections;
		PagedArray<RID> decals;
		PagedArray<RID> voxel_gi_instances;
		PagedArray<RID> mesh_instances;
		PagedArray<RID> fog_volumes;
		struct DirectionalShadow {
			PagedArray<RenderGeometryInstance *> cascade_geometry_instances[RendererSceneRender::MAX_DIRECTIONAL_LIGHT_CASCADES]; // 为啥这些都是Geometry
		} directional_shadows[RendererSceneRender::MAX_DIRECTIONAL_LIGHTS];
		PagedArray<RenderGeometryInstance *> sdfgi_region_geometry_instances[SDFGI_MAX_CASCADES * SDFGI_MAX_REGIONS_PER_CASCADE]; // 为啥不在scene renderer里
		PagedArray<RID> sdfgi_cascade_lights[SDFGI_MAX_CASCADES];

		void clear();
		void reset();
		void append_from(InstanceCullResult &p_cull_result);
		void init(PagedArrayPool<RID> *p_rid_pool, PagedArrayPool<RenderGeometryInstance *> *p_geometry_instance_pool, PagedArrayPool<Instance *> *p_instance_pool); // setpool
		// page array的实现允许用一个池
	};

	InstanceCullResult scene_cull_result;
	LocalVector<InstanceCullResult> scene_cull_result_threads;
	RendererSceneRender::RenderShadowData render_shadow_data[MAX_UPDATE_SHADOWS];
	uint32_t max_shadows_used = 0;
	uint32_t thread_cull_threshold = 200; // 多线程cull
	
	// RendererSceneRender::RenderSDFGIData render_sdfgi_data[SDFGI_MAX_CASCADES * SDFGI_MAX_REGIONS_PER_CASCADE];
	// RendererSceneRender::RenderSDFGIUpdateData sdfgi_update_data;

	RID_Owner<Instance, true> instance_owner;
	virtual RID instance_allocate();
	virtual void instance_initialize(RID p_rid);
virtual void instance_set_base(RID p_instance, RID p_base);
	virtual void instance_set_scenario(RID p_instance, RID p_scenario);
	virtual void instance_set_layer_mask(RID p_instance, uint32_t p_mask);
	virtual void instance_set_pivot_data(RID p_instance, float p_sorting_offset, bool p_use_aabb_center);
	virtual void instance_set_transform(RID p_instance, const Transform3D &p_transform);
	virtual void instance_attach_object_instance_id(RID p_instance, ObjectID p_id);
	virtual void instance_set_blend_shape_weight(RID p_instance, int p_shape, float p_weight);
	virtual void instance_set_surface_override_material(RID p_instance, int p_surface, RID p_material);
	virtual void instance_set_visible(RID p_instance, bool p_visible);
	virtual void instance_geometry_set_transparency(RID p_instance, float p_transparency);

	private:
	SelfList<Instance>::List _instance_update_list;
	void _instance_update_mesh_instance(Instance *p_instance);
	void _instance_queue_update(Instance *p_instance, bool p_update_aabb = false, bool p_update_dependencies= false);

};
}  // namespace lain
#endif