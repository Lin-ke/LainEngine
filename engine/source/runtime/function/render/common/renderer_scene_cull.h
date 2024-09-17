#ifndef RENDERER_SCENE_CULL_H
#define RENDERER_SCENE_CULL_H
#include "function/render/rendering_system/rendering_method_api.h"
#include "core/math/dynamic_bvh.h"
namespace lain {
	// RendererSceneCull: scene renderer implementation, does indexing and frustum culling using CPU
class RendererSceneCull : public RenderingMethod {
  public:
	static RendererSceneCull *singleton;
  struct Camera {
		enum Type {
			PERSPECTIVE,
			ORTHOGONAL,
			FRUSTUM
		};
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
	virtual void camera_set_transform(RID p_camera, const Transform3D &p_transform);
	virtual void camera_set_cull_mask(RID p_camera, uint32_t p_layers);
	virtual void camera_set_environment(RID p_camera, RID p_env);
	virtual void camera_set_camera_attributes(RID p_camera, RID p_attributes);
	virtual void camera_set_compositor(RID p_camera, RID p_compositor);
	virtual void camera_set_use_vertical_aspect(RID p_camera, bool p_enable);
	virtual bool is_camera(RID p_camera) const;

	/* OCCLUDER API */

	virtual RID occluder_allocate();
	virtual void occluder_initialize(RID p_occluder);
	virtual void occluder_set_mesh(RID p_occluder, const PackedVector3Array &p_vertices, const PackedInt32Array &p_indices);

	/* SCENARIO API */
	struct Instance; struct InstanceBounds; struct InstanceData; 
	struct Scenario {
		enum IndexerType {
			INDEXER_GEOMETRY, //for geometry
			INDEXER_VOLUMES, //for everything else
			INDEXER_MAX
		};

		DynamicBVH indexers[INDEXER_MAX];

		RID self;

		List<Instance *> directional_lights;
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
		VisibilityArray instance_visibility;

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

};
}
#endif