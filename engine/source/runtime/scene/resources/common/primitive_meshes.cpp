#include "primitive_meshes.h"
using namespace lain;

float PrimitiveMesh::get_lightmap_texel_size() const {
	float texel_size = GLOBAL_GET("rendering/lightmapping/primitive_meshes/texel_size");

	if (texel_size <= 0.0) {
		texel_size = 0.2;
	}

	return texel_size;
}
void lain::PrimitiveMesh::set_material(const Ref<Material>& p_material) {
  material = p_material;
	if (!pending_request) {
		// just apply it, else it'll happen when _update is called.
		RS::get_singleton()->mesh_surface_set_material(mesh, 0, material.is_null() ? RID() : material->GetRID());
		notify_property_list_changed();
		emit_changed();
	}
}
lain::PrimitiveMesh::PrimitiveMesh() {

  mesh = RS::get_singleton()->mesh_create();
}

void CapsuleMesh::_create_mesh_array(Array &p_arr) const {
	bool _add_uv2 = get_add_uv2();
	float texel_size = get_lightmap_texel_size();
	float _uv2_padding = get_uv2_padding() * texel_size;

	create_mesh_array(p_arr, radius, height, radial_segments, rings, _add_uv2, _uv2_padding);
}
void CapsuleMesh::create_mesh_array(Array& p_arr, const float radius, const float height, const int radial_segments, const int rings, bool p_add_uv2,
                                    const float p_uv2_padding) {
  int i, j, prevrow, thisrow, point;
  float x, y, z, u, v, w;
  float onethird = 1.0 / 3.0;
  float twothirds = 2.0 / 3.0;

  // Only used if we calculate UV2
  float radial_width = 2.0 * radius * Math_PI;
  float radial_h = radial_width / (radial_width + p_uv2_padding);
  float radial_length = radius * Math_PI * 0.5;                                         // circumference of 90 degree bend
  float vertical_length = radial_length * 2 + (height - 2.0 * radius) + p_uv2_padding;  // total vertical length
  float radial_v = radial_length / vertical_length;                                     // v size of top and bottom section
  float height_v = (height - 2.0 * radius) / vertical_length;                           // v size of height section

  // note, this has been aligned with our collision shape but I've left the descriptions as top/middle/bottom

  Vector<Vector3> points;
  Vector<Vector3> normals;
  Vector<float> tangents;
  Vector<Vector2> uvs;
  Vector<Vector2> uv2s;
  Vector<int> indices;
  point = 0;

#define ADD_TANGENT(m_x, m_y, m_z, m_d) \
	tangents.push_back(m_x);            \
	tangents.push_back(m_y);            \
	tangents.push_back(m_z);            \
	tangents.push_back(m_d);

	/* top hemisphere */
	thisrow = 0;
	prevrow = 0;
	for (j = 0; j <= (rings + 1); j++) {
		v = j;

		v /= (rings + 1);
		w = sin(0.5 * Math_PI * v);
		y = radius * cos(0.5 * Math_PI * v);

		for (i = 0; i <= radial_segments; i++) {
			u = i;
			u /= radial_segments;

			x = -sin(u * Math_TAU);
			z = cos(u * Math_TAU);

			Vector3 p = Vector3(x * radius * w, y, -z * radius * w);
			points.push_back(p + Vector3(0.0, 0.5 * height - radius, 0.0));
			normals.push_back(p.normalized());
			ADD_TANGENT(-z, 0.0, -x, 1.0)
			uvs.push_back(Vector2(u, v * onethird));
			if (p_add_uv2) {
				uv2s.push_back(Vector2(u * radial_h, v * radial_v));
			}
			point++;

			if (i > 0 && j > 0) {
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);

				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			}
		}

		prevrow = thisrow;
		thisrow = point;
	}

	/* cylinder */
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= (rings + 1); j++) {
		v = j;
		v /= (rings + 1);

		y = (height - 2.0 * radius) * v;
		y = (0.5 * height - radius) - y;

		for (i = 0; i <= radial_segments; i++) {
			u = i;
			u /= radial_segments;

			x = -sin(u * Math_TAU);
			z = cos(u * Math_TAU);

			Vector3 p = Vector3(x * radius, y, -z * radius);
			points.push_back(p);
			normals.push_back(Vector3(x, 0.0, -z));
			ADD_TANGENT(-z, 0.0, -x, 1.0)
			uvs.push_back(Vector2(u, onethird + (v * onethird)));
			if (p_add_uv2) {
				uv2s.push_back(Vector2(u * radial_h, radial_v + (v * height_v)));
			}
			point++;

			if (i > 0 && j > 0) {
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);

				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			}
		}

		prevrow = thisrow;
		thisrow = point;
	}

	/* bottom hemisphere */
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= (rings + 1); j++) {
		v = j;

		v /= (rings + 1);
		v += 1.0;
		w = sin(0.5 * Math_PI * v);
		y = radius * cos(0.5 * Math_PI * v);

		for (i = 0; i <= radial_segments; i++) {
			u = i;
			u /= radial_segments;

			x = -sin(u * Math_TAU);
			z = cos(u * Math_TAU);

			Vector3 p = Vector3(x * radius * w, y, -z * radius * w);
			points.push_back(p + Vector3(0.0, -0.5 * height + radius, 0.0));
			normals.push_back(p.normalized());
			ADD_TANGENT(-z, 0.0, -x, 1.0)
			uvs.push_back(Vector2(u, twothirds + ((v - 1.0) * onethird)));
			if (p_add_uv2) {
				uv2s.push_back(Vector2(u * radial_h, radial_v + height_v + ((v - 1.0) * radial_v)));
			}
			point++;

			if (i > 0 && j > 0) {
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);

				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			}
		}

		prevrow = thisrow;
		thisrow = point;
	}

	p_arr[RS::ARRAY_VERTEX] = points;
	p_arr[RS::ARRAY_NORMAL] = normals;
	p_arr[RS::ARRAY_TANGENT] = tangents;
	p_arr[RS::ARRAY_TEX_UV] = uvs;
	if (p_add_uv2) {
		p_arr[RS::ARRAY_TEX_UV2] = uv2s;
	}
	p_arr[RS::ARRAY_INDEX] = indices;
}

void PrimitiveMesh::_update() const {
	Array arr;
	// if (GDVIRTUAL_CALL(_create_mesh_array, arr)) {
	// 	ERR_FAIL_COND_MSG(arr.size() != RS::ARRAY_MAX, "_create_mesh_array must return an array of Mesh.ARRAY_MAX elements.");
	// } else {
		arr.resize(RS::ARRAY_MAX);
		_create_mesh_array(arr);
	// }

	Vector<Vector3> points = arr[RS::ARRAY_VERTEX];

	ERR_FAIL_COND_MSG(points.is_empty(), "_create_mesh_array must return at least a vertex array.");

	aabb = AABB();

	int pc = points.size();
	ERR_FAIL_COND(pc == 0);
	{
		const Vector3 *r = points.ptr();
		for (int i = 0; i < pc; i++) {
			if (i == 0) {
				aabb.position = r[i];
			} else {
				aabb.expand_to(r[i]);
			}
		}
	}

	Vector<int> indices = arr[RS::ARRAY_INDEX];

	if (flip_faces) {
		Vector<Vector3> normals = arr[RS::ARRAY_NORMAL];

		if (normals.size() && indices.size()) {
			{
				int nc = normals.size();
				Vector3 *w = normals.ptrw();
				for (int i = 0; i < nc; i++) {
					w[i] = -w[i];
				}
			}

			{
				int ic = indices.size();
				int *w = indices.ptrw();
				for (int i = 0; i < ic; i += 3) {
					SWAP(w[i + 0], w[i + 1]);
				}
			}
			arr[RS::ARRAY_NORMAL] = normals;
			arr[RS::ARRAY_INDEX] = indices;
		}
	}

	if (add_uv2) {
		// _create_mesh_array should populate our UV2, this is a fallback in case it doesn't.
		// As we don't know anything about the geometry we only pad the right and bottom edge
		// of our texture.
		Vector<Vector2> uv = arr[RS::ARRAY_TEX_UV];
		Vector<Vector2> uv2 = arr[RS::ARRAY_TEX_UV2];

		if (uv.size() > 0 && uv2.size() == 0) {
			Vector2 uv2_scale = get_uv2_scale();
			uv2.resize(uv.size());

			Vector2 *uv2w = uv2.ptrw();
			for (int i = 0; i < uv.size(); i++) {
				uv2w[i] = uv[i] * uv2_scale;
			}
		}

		arr[RS::ARRAY_TEX_UV2] = uv2;
	}

	array_len = pc;
	index_array_len = indices.size();
	// in with the new
	RS::get_singleton()->mesh_clear(mesh);
	RS::get_singleton()->mesh_add_surface_from_arrays(mesh, (RS::PrimitiveType)primitive_type, arr);
	RS::get_singleton()->mesh_surface_set_material(mesh, 0, material.is_null() ? RID() : material->GetRID());

	pending_request = false;

	clear_cache();

	const_cast<PrimitiveMesh *>(this)->emit_changed();
}
