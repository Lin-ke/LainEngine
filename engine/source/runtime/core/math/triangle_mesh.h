
#ifndef TRIANGLE_MESH_H
#define TRIANGLE_MESH_H

#include "core/math/face3.h"
#include "core/object/refcounted.h"
namespace lain {
	// 防止重名，一般只需要最后一层
	INNER_REFLECTION_TYPE(Triangle, TriangleMesh);
class TriangleMesh : public RefCounted {
	LCLASS(TriangleMesh, RefCounted);

public:
	
	STRUCT(Triangle, Fields){
		INNER_REFLECTION_BODY(Triangle, TriangleMesh);
		Vector3 normal;
		int indices[3];
		int32_t surface_index;
	};

private:
	Vector<Triangle> triangles;
	Vector<Vector3> vertices;

	struct BVH {
		AABB aabb;
		Vector3 center; //used for sorting
		int left;
		int right;

		int face_index;
	};

	struct BVHCmpX {
		bool operator()(const BVH* p_left, const BVH* p_right) const {
			return p_left->center.x < p_right->center.x;
		}
	};

	struct BVHCmpY {
		bool operator()(const BVH* p_left, const BVH* p_right) const {
			return p_left->center.y < p_right->center.y;
		}
	};
	struct BVHCmpZ {
		bool operator()(const BVH* p_left, const BVH* p_right) const {
			return p_left->center.z < p_right->center.z;
		}
	};

	int _create_bvh(BVH* p_bvh, BVH** p_bb, int p_from, int p_size, int p_depth, int& max_depth, int& max_alloc);

	Vector<BVH> bvh;
	int max_depth;
	bool valid;

public:
	bool is_valid() const;
	bool intersect_segment(const Vector3& p_begin, const Vector3& p_end, Vector3& r_point, Vector3& r_normal, int32_t* r_surf_index = nullptr) const;
	bool intersect_ray(const Vector3& p_begin, const Vector3& p_dir, Vector3& r_point, Vector3& r_normal, int32_t* r_surf_index = nullptr) const;
	bool inside_convex_shape(const Plane* p_planes, int p_plane_count, const Vector3* p_points, int p_point_count, Vector3 p_scale = Vector3(1, 1, 1)) const;
	Vector<Face3> get_faces() const;

	const Vector<Triangle>& get_triangles() const { return triangles; }
	const Vector<Vector3>& get_vertices() const { return vertices; }
	void get_indices(Vector<int>* r_triangles_indices) const;

	void create(const Vector<Vector3>& p_faces, const Vector<int32_t>& p_surface_indices = Vector<int32_t>());
	TriangleMesh();
};
}

#endif // TRIANGLE_MESH_H
