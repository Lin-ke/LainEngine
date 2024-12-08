#include "mesh.h"
using namespace lain;

Vector<Face3> lain::Mesh::get_faces() const {
  	Ref<TriangleMesh> tm = generate_triangle_mesh();
	if (tm.is_valid()) {
		return tm->get_faces();
	}
	return Vector<Face3>();
}