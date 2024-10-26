#include "rendering_light_culler.h"

bool lain::RenderingLightCuller::prepare_camera(const Transform3D& p_cam_transform, const Projection& p_cam_matrix) {
	data.debug_count++;
	if (data.debug_count >= 120) {
		data.debug_count = 0;
	}
if (!data.is_active()) {
		return false;
	}

	// Get the camera frustum planes in world space.
	data.frustum_planes = p_cam_matrix.get_projection_planes(p_cam_transform);
	// DEV_CHECK_ONCE(data.frustum_planes.size() == 6);

	data.regular_cull_planes.num_cull_planes = 0;
	data.directional_cull_planes.resize(0);


	// We want to calculate the frustum corners in a specific order.
	const Projection::Planes intersections[8][3] = {
		{ Projection::PLANE_FAR, Projection::PLANE_LEFT, Projection::PLANE_TOP },
		{ Projection::PLANE_FAR, Projection::PLANE_LEFT, Projection::PLANE_BOTTOM },
		{ Projection::PLANE_FAR, Projection::PLANE_RIGHT, Projection::PLANE_TOP },
		{ Projection::PLANE_FAR, Projection::PLANE_RIGHT, Projection::PLANE_BOTTOM },
		{ Projection::PLANE_NEAR, Projection::PLANE_LEFT, Projection::PLANE_TOP },
		{ Projection::PLANE_NEAR, Projection::PLANE_LEFT, Projection::PLANE_BOTTOM },
		{ Projection::PLANE_NEAR, Projection::PLANE_RIGHT, Projection::PLANE_TOP },
		{ Projection::PLANE_NEAR, Projection::PLANE_RIGHT, Projection::PLANE_BOTTOM },
	};
  for (int i = 0; i < 8; i++) {
      // 3 plane intersection, gives us a point.
      bool res = data.frustum_planes[intersections[i][0]].intersect_3(data.frustum_planes[intersections[i][1]], data.frustum_planes[intersections[i][2]], &data.frustum_points[i]);

      // What happens with a zero frustum? NYI - deal with this.
      ERR_FAIL_COND_V(!res, false);

  #ifdef LIGHT_CULLER_DEBUG_LOGGING
      if (is_logging()) {
        print_line("point " + itos(i) + " -> " + String(data.frustum_points[i]));
      }
  #endif
    }

	return true;
}