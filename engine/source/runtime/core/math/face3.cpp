#include "face3.h"
namespace lain {
	Plane Face3::get_plane(ClockDirection p_dir) const {
		return Plane(vertex[0], vertex[1], vertex[2], p_dir);
	}

	bool Face3::intersects_ray(const Vector3& p_from, const Vector3& p_dir, Vector3* p_intersection) const {
		//return Geometry3D::ray_intersects_triangle(p_from, p_dir, vertex[0], vertex[1], vertex[2], p_intersection);
		///@TODO
		return false;
	}
	bool Face3::intersects_segment(const Vector3& p_from, const Vector3& p_dir, Vector3* p_intersection) const {
		//return Geometry3D::segment_intersects_triangle(p_from, p_dir, vertex[0], vertex[1], vertex[2], p_intersection);
	///@todo
		return false;
	}

}