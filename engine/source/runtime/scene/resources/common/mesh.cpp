#include "mesh.h"
using namespace lain;

Vector<Face3> lain::Mesh::get_faces() const {
  	Ref<TriangleMesh> tm = generate_triangle_mesh();
	if (tm.is_valid()) {
		return tm->get_faces();
	}
	return Vector<Face3>();
}

Vector<Face3> lain::Mesh::get_surface_faces(int p_surface) const {
	Ref<TriangleMesh> tm = generate_surface_triangle_mesh(p_surface);
	if (tm.is_valid()) {
		return tm->get_faces();
	}
	return Vector<Face3>();
}

Ref<TriangleMesh> Mesh::generate_triangle_mesh() const {
	if (triangle_mesh.is_valid()) {
		return triangle_mesh;
	}

	int faces_size = 0;

	for (int i = 0; i < get_surface_count(); i++) {
		switch (surface_get_primitive_type(i)) {
			case PRIMITIVE_TRIANGLES: {
				int len = (surface_get_format(i) & ARRAY_FORMAT_INDEX) ? surface_get_array_index_len(i) : surface_get_array_len(i);
				// Don't error if zero, it's valid (we'll just skip it later).
				ERR_CONTINUE_MSG((len % 3) != 0, vformat("Ignoring surface %d, incorrect %s count: %d (for PRIMITIVE_TRIANGLES).", i, (surface_get_format(i) & ARRAY_FORMAT_INDEX) ? "index" : "vertex", len));
				faces_size += len;
			} break;
			case PRIMITIVE_TRIANGLE_STRIP: {
				int len = (surface_get_format(i) & ARRAY_FORMAT_INDEX) ? surface_get_array_index_len(i) : surface_get_array_len(i);
				// Don't error if zero, it's valid (we'll just skip it later).
				ERR_CONTINUE_MSG(len != 0 && len < 3, vformat("Ignoring surface %d, incorrect %s count: %d (for PRIMITIVE_TRIANGLE_STRIP).", i, (surface_get_format(i) & ARRAY_FORMAT_INDEX) ? "index" : "vertex", len));
				faces_size += (len == 0) ? 0 : (len - 2) * 3;
			} break;
			default: {
			} break;
		}
	}

	if (faces_size == 0) {
		return triangle_mesh;
	}

	Vector<Vector3> faces;
	faces.resize(faces_size);
	Vector<int32_t> surface_indices;
	surface_indices.resize(faces_size / 3);
	Vector3 *facesw = faces.ptrw();
	int32_t *surface_indicesw = surface_indices.ptrw();

	int widx = 0;

	for (int i = 0; i < get_surface_count(); i++) {
		Mesh::PrimitiveType primitive = surface_get_primitive_type(i);
		if (primitive != PRIMITIVE_TRIANGLES && primitive != PRIMITIVE_TRIANGLE_STRIP) {
			continue;
		}
		int len = (surface_get_format(i) & ARRAY_FORMAT_INDEX) ? surface_get_array_index_len(i) : surface_get_array_len(i);
		if ((primitive == PRIMITIVE_TRIANGLES && (len == 0 || (len % 3) != 0)) ||
				(primitive == PRIMITIVE_TRIANGLE_STRIP && len < 3) ||
				(surface_get_format(i) & ARRAY_FLAG_USES_EMPTY_VERTEX_ARRAY)) {
			// Error was already shown, just skip (including zero).
			continue;
		}

		Array a = surface_get_arrays(i);
		ERR_FAIL_COND_V(a.is_empty(), Ref<TriangleMesh>());

		int vc = surface_get_array_len(i);
		Vector<Vector3> vertices = a[ARRAY_VERTEX];
		ERR_FAIL_COND_V(vertices.is_empty(), Ref<TriangleMesh>());
		const Vector3 *vr = vertices.ptr();

		int32_t from_index = widx / 3;

		if (surface_get_format(i) & ARRAY_FORMAT_INDEX) {
			int ic = surface_get_array_index_len(i);
			Vector<int> indices = a[ARRAY_INDEX];
			const int *ir = indices.ptr();

			if (primitive == PRIMITIVE_TRIANGLES) {
				for (int j = 0; j < ic; j++) {
					int index = ir[j];
					ERR_FAIL_COND_V(index >= vc, Ref<TriangleMesh>());
					facesw[widx++] = vr[index];
				}
			} else { // PRIMITIVE_TRIANGLE_STRIP
				for (int j = 2; j < ic; j++) {
					facesw[widx++] = vr[ir[j - 2]];
					facesw[widx++] = vr[ir[j - 1]];
					facesw[widx++] = vr[ir[j]];
				}
			}

		} else {
			if (primitive == PRIMITIVE_TRIANGLES) {
				for (int j = 0; j < vc; j++) {
					facesw[widx++] = vr[j];
				}
			} else { // PRIMITIVE_TRIANGLE_STRIP
				for (int j = 2; j < vc; j++) {
					facesw[widx++] = vr[j - 2];
					facesw[widx++] = vr[j - 1];
					facesw[widx++] = vr[j];
				}
			}
		}

		int32_t to_index = widx / 3;

		for (int j = from_index; j < to_index; j++) {
			surface_indicesw[j] = i;
		}
	}

	triangle_mesh = Ref<TriangleMesh>(memnew(TriangleMesh));
	triangle_mesh->create(faces);

	return triangle_mesh;
}

Ref<TriangleMesh> lain::Mesh::generate_surface_triangle_mesh(int p_surface) const {
  ERR_FAIL_INDEX_V(p_surface, get_surface_count(), Ref<TriangleMesh>());

	if (surface_triangle_meshes.size() != get_surface_count()) {
		surface_triangle_meshes.resize(get_surface_count());
	}

	if (surface_triangle_meshes[p_surface].is_valid()) {
		return surface_triangle_meshes[p_surface];
	}

	int facecount = 0;

	if (surface_get_primitive_type(p_surface) != PRIMITIVE_TRIANGLES) {
		return Ref<TriangleMesh>();
	}

	if (surface_get_format(p_surface) & ARRAY_FORMAT_INDEX) {
		facecount += surface_get_array_index_len(p_surface);
	} else {
		facecount += surface_get_array_len(p_surface);
	}

	Vector<Vector3> faces;
	faces.resize(facecount);
	Vector3 *facesw = faces.ptrw();

	Array a = surface_get_arrays(p_surface);
	ERR_FAIL_COND_V(a.is_empty(), Ref<TriangleMesh>());

	int vc = surface_get_array_len(p_surface);
	Vector<Vector3> vertices = a[ARRAY_VERTEX];
	const Vector3 *vr = vertices.ptr();
	int widx = 0;

	if (surface_get_format(p_surface) & ARRAY_FORMAT_INDEX) {
		int ic = surface_get_array_index_len(p_surface);
		Vector<int> indices = a[ARRAY_INDEX];
		const int *ir = indices.ptr();

		for (int j = 0; j < ic; j++) {
			int index = ir[j];
			facesw[widx++] = vr[index];
		}

	} else {
		for (int j = 0; j < vc; j++) {
			facesw[widx++] = vr[j];
		}
	}

	Ref<TriangleMesh> tr_mesh = Ref<TriangleMesh>(memnew(TriangleMesh));
	tr_mesh->create(faces);
	surface_triangle_meshes.set(p_surface, tr_mesh);

	return tr_mesh;
}

void lain::Mesh::generate_debug_mesh_lines(Vector<Vector3>& r_lines) {
		if (debug_lines.size() > 0) {
		r_lines = debug_lines;
		return;
	}

	Ref<TriangleMesh> tm = generate_triangle_mesh();
	if (tm.is_null()) {
		return;
	}

	Vector<int> triangle_indices;
	tm->get_indices(&triangle_indices);
	const int triangles_num = tm->get_triangles().size();
	Vector<Vector3> vertices = tm->get_vertices();

	debug_lines.resize(tm->get_triangles().size() * 6); // 3 lines x 2 points each line

	const int *ind_r = triangle_indices.ptr();
	const Vector3 *ver_r = vertices.ptr();
	for (int j = 0, x = 0, i = 0; i < triangles_num; j += 6, x += 3, ++i) {
		// Triangle line 1
		debug_lines.write[j + 0] = ver_r[ind_r[x + 0]];
		debug_lines.write[j + 1] = ver_r[ind_r[x + 1]];

		// Triangle line 2
		debug_lines.write[j + 2] = ver_r[ind_r[x + 1]];
		debug_lines.write[j + 3] = ver_r[ind_r[x + 2]];

		// Triangle line 3
		debug_lines.write[j + 4] = ver_r[ind_r[x + 2]];
		debug_lines.write[j + 5] = ver_r[ind_r[x + 0]];
	}

	r_lines = debug_lines;
}

int Mesh::get_surface_count() const {
  return 0;
}

int lain::Mesh::surface_get_array_len(int p_idx) const {
  return 0;
}

int lain::Mesh::surface_get_array_index_len(int p_idx) const {
  return 0;
}

Array lain::Mesh::surface_get_arrays(int p_surface) const {
  return Array();
}

Vector<Variant> lain::Mesh::surface_get_blend_shape_arrays(int p_surface) const {
  return Vector<Variant>();
}

Dictionary lain::Mesh::surface_get_lods(int p_surface) const {
  return Dictionary();
}

BitField<Mesh::ArrayFormat> lain::Mesh::surface_get_format(int p_idx) const {
	uint32_t ret = 0;
	return ret;
}

Mesh::PrimitiveType lain::Mesh::surface_get_primitive_type(int p_idx) const {
  return PrimitiveType();
}

void lain::Mesh::surface_set_material(int p_idx, const Ref<Material>& p_material) {}

Ref<Material> lain::Mesh::surface_get_material(int p_idx) const {
  return Ref<Material>();
}

int lain::Mesh::get_blend_shape_count() const {
  return 0;
}

StringName lain::Mesh::get_blend_shape_name(int p_index) const {
  return StringName();
}

void lain::Mesh::set_blend_shape_name(int p_index, const StringName& p_name) {}

AABB lain::Mesh::get_aabb() const {
  return AABB();
}

void Mesh::generate_debug_mesh_indices(Vector<Vector3> &r_points) {
	Ref<TriangleMesh> tm = generate_triangle_mesh();
	if (tm.is_null()) {
		return;
	}

	Vector<Vector3> vertices = tm->get_vertices();

	int vertices_size = vertices.size();
	r_points.resize(vertices_size);
	for (int i = 0; i < vertices_size; ++i) {
		r_points.write[i] = vertices[i]; // @opt
	}


}

Size2i lain::Mesh::get_lightmap_size_hint() const {
  return lightmap_size_hint;
}

void Mesh::clear_cache() const {
	triangle_mesh.unref();
	debug_lines.clear();
}