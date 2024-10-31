#include "rendering_light_culler.h"
#include "function/render/rendering_system/rendering_system_globals.h"
using namespace lain;
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
      {Projection::PLANE_FAR, Projection::PLANE_LEFT, Projection::PLANE_TOP},   {Projection::PLANE_FAR, Projection::PLANE_LEFT, Projection::PLANE_BOTTOM},
      {Projection::PLANE_FAR, Projection::PLANE_RIGHT, Projection::PLANE_TOP},  {Projection::PLANE_FAR, Projection::PLANE_RIGHT, Projection::PLANE_BOTTOM},
      {Projection::PLANE_NEAR, Projection::PLANE_LEFT, Projection::PLANE_TOP},  {Projection::PLANE_NEAR, Projection::PLANE_LEFT, Projection::PLANE_BOTTOM},
      {Projection::PLANE_NEAR, Projection::PLANE_RIGHT, Projection::PLANE_TOP}, {Projection::PLANE_NEAR, Projection::PLANE_RIGHT, Projection::PLANE_BOTTOM},
  };
  for (int i = 0; i < 8; i++) {
    // 3 plane intersection, gives us a point.
    bool res =
        data.frustum_planes[intersections[i][0]].intersect_3(data.frustum_planes[intersections[i][1]], data.frustum_planes[intersections[i][2]], &data.frustum_points[i]);

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

void lain::RenderingLightCuller::prepare_directional_light(const RendererSceneCull::Instance* p_instance, int32_t p_directional_light_id) {

  //data.directional_light = p_instance;
  // Something is probably going wrong, we shouldn't have this many directional lights...
  ERR_FAIL_COND(p_directional_light_id > 1024);
  DEV_ASSERT(p_directional_light_id >= 0);

  // First make sure we have enough directional lights to hold this one.
  if (p_directional_light_id >= (int32_t)data.directional_cull_planes.size()) {
    data.directional_cull_planes.resize(p_directional_light_id + 1);
  }

  _prepare_light(*p_instance, p_directional_light_id, RS::LIGHT_DIRECTIONAL);
}

bool lain::RenderingLightCuller::cull_directional_light(const RendererSceneCull::InstanceBounds& p_bound, int32_t p_directional_light_id) {
	if (!data.is_active() || !is_caster_culling_active()) {
		return true;
	}

	ERR_FAIL_INDEX_V(p_directional_light_id, (int32_t)data.directional_cull_planes.size(), true);

	LightCullPlanes &cull_planes = data.directional_cull_planes[p_directional_light_id];

	Vector3 mins = Vector3(p_bound.bounds[0], p_bound.bounds[1], p_bound.bounds[2]);
	Vector3 maxs = Vector3(p_bound.bounds[3], p_bound.bounds[4], p_bound.bounds[5]);
	AABB bb(mins, maxs - mins);

	real_t r_min, r_max;
	for (int p = 0; p < cull_planes.num_cull_planes; p++) {
		bb.project_range_in_plane(cull_planes.cull_planes[p], r_min, r_max);
		if (r_min > 0.0f) {
#ifdef LIGHT_CULLER_DEBUG_DIRECTIONAL_LIGHT
			cull_planes.rejected_count++;
#endif

			return false;
		}
	}

	return true;
}

bool lain::RenderingLightCuller::_prepare_light(const RendererSceneCull::Instance& p_instance, int32_t p_directional_light_id, RS::LightType p_light_type) {
  if (!data.is_active()) {
    return true;
  }

  LightSource lsource;
  // 这里不知道light的实现细节
  // 可以 公开 一部分light 的接口，以避免频繁的调用 Light_get_param
  switch (p_light_type) {
    case RS::LIGHT_SPOT:
      lsource.type = LightSource::ST_SPOTLIGHT;
      lsource.angle = RSG::light_storage->light_get_param(p_instance.base, RS::LIGHT_PARAM_SPOT_ANGLE);
      lsource.range = RSG::light_storage->light_get_param(p_instance.base, RS::LIGHT_PARAM_RANGE);
      break;
    case RS::LIGHT_OMNI:
      lsource.type = LightSource::ST_OMNI;
      lsource.range = RSG::light_storage->light_get_param(p_instance.base, RS::LIGHT_PARAM_RANGE);
      break;
    case RS::LIGHT_DIRECTIONAL:
      lsource.type = LightSource::ST_DIRECTIONAL;
      // Could deal with a max directional shadow range here? NYI
      // LIGHT_PARAM_SHADOW_MAX_DISTANCE
      break;
  }

  lsource.pos = p_instance.transform.origin;
  lsource.dir = -p_instance.transform.basis.get_column(2);
  lsource.dir.normalize();

  bool visible;
  if (p_directional_light_id == -1) {
    visible = _add_light_camera_planes(data.regular_cull_planes, lsource);
  } else {
    visible = _add_light_camera_planes(data.directional_cull_planes[p_directional_light_id], lsource);
  }

  if (data.light_culling_active) {
    return visible;
  }
  return true;
}


bool RenderingLightCuller::_add_light_camera_planes(LightCullPlanes &r_cull_planes, const LightSource &p_light_source) {
	if (!data.is_active()) {
		return true;
	}

	// We should have called prepare_camera before this.
	ERR_FAIL_COND_V(data.frustum_planes.size() != 6, true);

	switch (p_light_source.type) {
		case LightSource::ST_SPOTLIGHT:
		case LightSource::ST_OMNI:
			break;
		case LightSource::ST_DIRECTIONAL:
			return add_light_camera_planes_directional(r_cull_planes, p_light_source);
			break;
		default:
			return false; // not yet supported
			break;
	}

	// Start with 0 cull planes.
	r_cull_planes.num_cull_planes = 0;
	data.out_of_range = false;
	uint32_t lookup = 0;

	// Find which of the camera planes are facing away from the light.
	// We can also test for the situation where the light max range means it cannot
	// affect the camera frustum. This is absolutely worth doing because it is relatively
	// cheap, and if the entire light can be culled this can vastly improve performance
	// (much more than just culling casters).

	// POINT LIGHT (spotlight, omni)
	// Instead of using dot product to compare light direction to plane, we can simply
	// find out which side of the plane the camera is on. By definition this marks the point at which the plane
	// becomes invisible.

	// OMNIS
	if (p_light_source.type == LightSource::ST_OMNI) {
		for (int n = 0; n < 6; n++) {
			float dist = data.frustum_planes[n].distance_to(p_light_source.pos);
			if (dist < 0.0f) {
				lookup |= 1 << n;

				// Add backfacing camera frustum planes.
				r_cull_planes.add_cull_plane(data.frustum_planes[n]);
			} else {
				// Is the light out of range?
				// This is one of the tests. If the point source is more than range distance from a frustum plane, it can't
				// be seen.
				if (dist >= p_light_source.range) {
					// If the light is out of range, no need to do anything else, everything will be culled.
					data.out_of_range = true;
					return false;
				}
			}
		}
	} else {
		// SPOTLIGHTs, more complex to cull.
		Vector3 pos_end = p_light_source.pos + (p_light_source.dir * p_light_source.range);

		// This is the radius of the cone at distance 1.
		float radius_at_dist_one = Math::tan(Math::deg_to_rad(p_light_source.angle));

		// The worst case radius of the cone at the end point can be calculated
		// (the radius will scale linearly with length along the cone).
		float end_cone_radius = radius_at_dist_one * p_light_source.range;

		for (int n = 0; n < 6; n++) {
			float dist = data.frustum_planes[n].distance_to(p_light_source.pos);
			if (dist < 0.0f) {
				// Either the plane is backfacing or we are inside the frustum.
				lookup |= 1 << n;

				// Add backfacing camera frustum planes.
				r_cull_planes.add_cull_plane(data.frustum_planes[n]);
			} else {
				// The light is in front of the plane.

				// Is the light out of range?
				if (dist >= p_light_source.range) {
					data.out_of_range = true;
					return false;
				}

				// For a spotlight, we can use an extra test
				// at this point the cone start is in front of the plane...
				// If the cone end point is further than the maximum possible distance to the plane
				// we can guarantee that the cone does not cross the plane, and hence the cone
				// is outside the frustum.
        // 点到平面的距离大于底部的圆锥半径，说明圆锥在平面外
				float dist_end = data.frustum_planes[n].distance_to(pos_end);

				if (dist_end >= end_cone_radius) {
					data.out_of_range = true;
					return false;
				}
			}
		}
	}

	// The lookup should be within the LUT, logic should prevent this.
  //2^6-1 = 63
	ERR_FAIL_COND_V(lookup >= LUT_SIZE, true);

	// Deal with special case... if the light is INSIDE the view frustum (i.e. all planes face away)
	// then we will add the camera frustum planes to clip the light volume .. there is no need to
	// render shadow casters outside the frustum as shadows can never re-enter the frustum.
  //  viewing frustum 的法线是向外的，如果distance 为 负数说明点在视锥内， 
	if (lookup == 63) {
		r_cull_planes.num_cull_planes = 0;
		for (int n = 0; n < data.frustum_planes.size(); n++) {
			r_cull_planes.add_cull_plane(data.frustum_planes[n]);
		}

		return true;
	}

	// Each edge forms a plane.
	uint8_t *entry = &data.LUT_entries[lookup][0];
	int n_edges = data.LUT_entry_sizes[lookup] - 1;

	const Vector3 &pt2 = p_light_source.pos;

	for (int e = 0; e < n_edges; e++) {
		int i0 = entry[e];
		int i1 = entry[e + 1];
		const Vector3 &pt0 = data.frustum_points[i0];
		const Vector3 &pt1 = data.frustum_points[i1];

		if (!_is_colinear_tri(pt0, pt1, pt2)) {
			// Create plane from 3 points.
			Plane p(pt0, pt1, pt2);
			r_cull_planes.add_cull_plane(p);
		}
	}

	// Last to 0 edge.
	if (n_edges) {
		int i0 = entry[n_edges]; // Last.
		int i1 = entry[0]; // First.

		const Vector3 &pt0 = data.frustum_points[i0];
		const Vector3 &pt1 = data.frustum_points[i1];

		if (!_is_colinear_tri(pt0, pt1, pt2)) {
			// Create plane from 3 points.
			Plane p(pt0, pt1, pt2);
			r_cull_planes.add_cull_plane(p);
		}
	}

#ifdef LIGHT_CULLER_DEBUG_LOGGING
	if (is_logging()) {
		print_line("lsource.pos is " + String(p_light_source.pos));
	}
#endif

	return true;
}

bool lain::RenderingLightCuller::add_light_camera_planes_directional(LightCullPlanes& r_cull_planes, const LightSource& p_light_source) {
uint32_t lookup = 0;
	r_cull_planes.num_cull_planes = 0;

	// Directional light, we will use dot against the light direction to determine back facing planes.
	for (int n = 0; n < 6; n++) {
		float dot = data.frustum_planes[n].normal.dot(p_light_source.dir);
		if (dot > 0.0f) {
			lookup |= 1 << n;

			// Add backfacing camera frustum planes.
			r_cull_planes.add_cull_plane(data.frustum_planes[n]);
		}
	}

	ERR_FAIL_COND_V(lookup >= LUT_SIZE, true);

	// Deal with special case... if the light is INSIDE the view frustum (i.e. all planes face away)
	// then we will add the camera frustum planes to clip the light volume .. there is no need to
	// render shadow casters outside the frustum as shadows can never re-enter the frustum.

	// Should never happen with directional light?? This may be able to be removed.
	if (lookup == 63) {
		r_cull_planes.num_cull_planes = 0;
		for (int n = 0; n < data.frustum_planes.size(); n++) {
			r_cull_planes.add_cull_plane(data.frustum_planes[n]);
		}

		return true;
	}

// Each edge forms a plane.
#ifdef RENDERING_LIGHT_CULLER_CALCULATE_LUT
	const LocalVector<uint8_t> &entry = _calculated_LUT[lookup];

	// each edge forms a plane
	int n_edges = entry.size() - 1;
#else
	uint8_t *entry = &data.LUT_entries[lookup][0];
	int n_edges = data.LUT_entry_sizes[lookup] - 1;
#endif

	for (int e = 0; e < n_edges; e++) {
		int i0 = entry[e];
		int i1 = entry[e + 1];
		const Vector3 &pt0 = data.frustum_points[i0];
		const Vector3 &pt1 = data.frustum_points[i1];

		// Create a third point from the light direction.
		Vector3 pt2 = pt0 - p_light_source.dir;

		if (!_is_colinear_tri(pt0, pt1, pt2)) {
			// Create plane from 3 points.
			Plane p(pt0, pt1, pt2);
			r_cull_planes.add_cull_plane(p);
		}
	}

	// Last to 0 edge.
	if (n_edges) {
		int i0 = entry[n_edges]; // Last.
		int i1 = entry[0]; // First.

		const Vector3 &pt0 = data.frustum_points[i0];
		const Vector3 &pt1 = data.frustum_points[i1];

		// Create a third point from the light direction.
		Vector3 pt2 = pt0 - p_light_source.dir;

		if (!_is_colinear_tri(pt0, pt1, pt2)) {
			// Create plane from 3 points.
			Plane p(pt0, pt1, pt2);
			r_cull_planes.add_cull_plane(p);
		}
	}

#ifdef LIGHT_CULLER_DEBUG_LOGGING
	if (is_logging()) {
		print_line("lcam.pos is " + String(p_light_source.pos));
	}
#endif

	return true;
}
