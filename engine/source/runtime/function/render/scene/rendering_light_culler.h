#ifndef RENDERINGLIGHTCULLER_H
#define RENDERINGLIGHTCULLER_H
#include "core/math/plane.h"
#include "core/math/vector3.h"
#include "renderer_scene_cull.h"
namespace lain {
class RenderingLightCuller {
 public:
  RenderingLightCuller();
  struct LightSource {
   public:
    enum SourceType {
      ST_UNKNOWN,
      ST_DIRECTIONAL,
      ST_SPOTLIGHT,
      ST_OMNI,
    };

    LightSource() {
      type = ST_UNKNOWN;
      angle = 0.0f;
      range = FLT_MAX;
    }

    // All in world space, culling done in world space.
    Vector3 pos;
    Vector3 dir;
    SourceType type;

    float angle;  // For spotlight.
    float range;
  };

  	// 6 bits, 6 planes.
	enum {
		NUM_CAM_PLANES = 6,
		NUM_CAM_POINTS = 8,
		MAX_CULL_PLANES = 17,
		LUT_SIZE = 64,
	};
  // Before each pass with a different camera, you must call this so the culler can pre-create
  // the camera frustum planes and corner points in world space which are used for the culling.
  bool prepare_camera(const Transform3D& p_cam_transform, const Projection& p_cam_matrix);

  // REGULAR LIGHTS (SPOT, OMNI).
	// These are prepared then used for culling one by one, single threaded.
	// prepare_regular_light() returns false if the entire light is culled (i.e. there is no intersection between the light and the view frustum).
	bool prepare_regular_light(const RendererSceneCull::Instance &p_instance) { return _prepare_light(p_instance, -1); }

	// Cull according to the regular light planes that were setup in the previous call to prepare_regular_light.
	void cull_regular_light(PagedArray<RendererSceneCull::Instance *> &r_instance_shadow_cull_result);

	// Directional lights are prepared in advance, and can be culled multithreaded chopping and changing between
	// different directional_light_id.
	void prepare_directional_light(const RendererSceneCull::Instance *p_instance, int32_t p_directional_light_id);

	// Return false if the instance is to be culled.
	bool cull_directional_light(const RendererSceneCull::InstanceBounds &p_bound, int32_t p_directional_light_id);

	// Can turn on and off from the engine if desired.
	void set_caster_culling_active(bool p_active) { data.caster_culling_active = p_active; }
	void set_light_culling_active(bool p_active) { data.light_culling_active = p_active; }
  private:
	struct LightCullPlanes {
		void add_cull_plane(const Plane &p);
		Plane cull_planes[MAX_CULL_PLANES];
		int num_cull_planes = 0;
#ifdef LIGHT_CULLER_DEBUG_DIRECTIONAL_LIGHT
		uint32_t rejected_count = 0;
#endif
	};
	bool _prepare_light(const RendererSceneCull::Instance &p_instance, int32_t p_directional_light_id = -1);
struct Data {
		// Camera frustum planes (world space) - order ePlane.
		Vector<Plane> frustum_planes;

		// Camera frustum corners (world space) - order ePoint.
		Vector3 frustum_points[NUM_CAM_POINTS];

		// Master can have multiple directional lights.
		// These need to store their own cull planes individually, as master
		// chops and changes between culling different lights
		// instead of doing one by one, and we don't want to prepare
		// lights multiple times per frame.
		LocalVector<LightCullPlanes> directional_cull_planes;

		// Single threaded cull planes for regular lights
		// (OMNI, SPOT). These lights reuse the same set of cull plane data.
		LightCullPlanes regular_cull_planes;

#ifdef LIGHT_CULLER_DEBUG_REGULAR_LIGHT
		uint32_t regular_rejected_count = 0;
#endif
		// The whole regular light can be out of range of the view frustum, in which case all casters should be culled.
		bool out_of_range = false;

#ifdef RENDERING_LIGHT_CULLER_DEBUG_STRINGS
		static String plane_bitfield_to_string(unsigned int BF);
		// Names of the plane and point enums, useful for debugging.
		static const char *string_planes[];
		static const char *string_points[];
#endif

		// Precalculated look up table.
		static uint8_t LUT_entry_sizes[LUT_SIZE];
		static uint8_t LUT_entries[LUT_SIZE][8];

		bool caster_culling_active = true;
		bool light_culling_active = true;

		// Light culling is a basic on / off switch.
		// Caster culling only works if light culling is also on.
		bool is_active() const { return light_culling_active; }

		// Ideally a frame counter, but for ease of implementation
		// this is just incremented on each prepare_camera.
		// used to turn on and off debugging features.
		int debug_count = -1;
	} data;
};
}  // namespace lain
#endif