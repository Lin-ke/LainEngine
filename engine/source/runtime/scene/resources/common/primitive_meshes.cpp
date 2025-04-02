#include "primitive_meshes.h"
using namespace lain;
#define PADDING_REF_SIZE 1024.0

Vector2 PrimitiveMesh::get_uv2_scale(Vector2 p_margin_scale) const {
	Vector2 uv2_scale;
	Vector2 lightmap_size = get_lightmap_size_hint();

	// Calculate it as a margin, if no lightmap size hint is given we assume "PADDING_REF_SIZE" as our texture size.
	uv2_scale.x = p_margin_scale.x * uv2_padding / (lightmap_size.x == 0.0 ? PADDING_REF_SIZE : lightmap_size.x);
	uv2_scale.y = p_margin_scale.y * uv2_padding / (lightmap_size.y == 0.0 ? PADDING_REF_SIZE : lightmap_size.y);

	// Inverse it to turn our margin into a scale
	uv2_scale = Vector2(1.0, 1.0) - uv2_scale;

	return uv2_scale;
}

float PrimitiveMesh::get_lightmap_texel_size() const {
  float texel_size = GLOBAL_GET("rendering/lightmapping/primitive_meshes/texel_size");

  if (texel_size <= 0.0) {
    texel_size = 0.2;
  }

  return texel_size;
}
void lain::PrimitiveMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_material", "material"), &PrimitiveMesh::set_material);
	ClassDB::bind_method(D_METHOD("get_material"), &PrimitiveMesh::get_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "BaseMaterial3D,ShaderMaterial"), "set_material", "get_material");
}
int lain::PrimitiveMesh::get_surface_count() const {
  if (pending_request) {
    _update();
  }
  return 1;
}

int PrimitiveMesh::surface_get_array_len(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, 1, -1);
	if (pending_request) {
		_update();
	}

	return array_len;
}

int lain::PrimitiveMesh::surface_get_array_index_len(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, 1, -1);
	if (pending_request) {
		_update();
	}

	return index_array_len;
}

Dictionary lain::PrimitiveMesh::surface_get_lods(int p_surface) const {
	return Dictionary(); //not really supported
}

BitField<Mesh::ArrayFormat> lain::PrimitiveMesh::surface_get_format(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, 1, 0);

	uint64_t mesh_format = RS::ARRAY_FORMAT_VERTEX | RS::ARRAY_FORMAT_NORMAL | RS::ARRAY_FORMAT_TANGENT | RS::ARRAY_FORMAT_TEX_UV | RS::ARRAY_FORMAT_INDEX;
	if (add_uv2) {
		mesh_format |= RS::ARRAY_FORMAT_TEX_UV2;
	}

	return mesh_format;
}
Mesh::PrimitiveType PrimitiveMesh::surface_get_primitive_type(int p_idx) const {
	return primitive_type;
}

void PrimitiveMesh::surface_set_material(int p_idx, const Ref<Material> &p_material) {
	ERR_FAIL_INDEX(p_idx, 1);

	set_material(p_material);
}

Ref<Material> PrimitiveMesh::surface_get_material(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, 1, nullptr);

	return material;
}
StringName PrimitiveMesh::get_blend_shape_name(int p_index) const {
	return StringName();
}

void PrimitiveMesh::set_blend_shape_name(int p_index, const StringName &p_name) {
}

AABB PrimitiveMesh::get_aabb() const {
	if (pending_request) {
		_update();
	}

	return aabb;
}
RID PrimitiveMesh::GetRID() const {
	if (pending_request) {
		_update();
	}
	return mesh;
}
int PrimitiveMesh::get_blend_shape_count() const {
	return 0;
}
Array PrimitiveMesh::surface_get_arrays(int p_surface) const {
	ERR_FAIL_INDEX_V(p_surface, 1, Array());
	if (pending_request) {
		_update();
	}
	return RS::get_singleton()->mesh_surface_get_arrays(mesh, 0);
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

Ref<Material> PrimitiveMesh::get_material() const {
	return material;
}

void lain::PrimitiveMesh::request_update()
{	if (pending_request) {
		return;
	}
	_update();
}

lain::PrimitiveMesh::PrimitiveMesh() {

  mesh = RS::get_singleton()->mesh_create();
}

PrimitiveMesh::~PrimitiveMesh() {
	ERR_FAIL_NULL(RS::get_singleton());
	RS::get_singleton()->free(mesh);
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

void CapsuleMesh::set_radius(const float p_radius) {
	radius = p_radius;
	if (radius > height * 0.5) {
		height = radius * 2.0;
	}
	_update_lightmap_size();
	request_update();
}

float lain::CapsuleMesh::get_radius() const {
return radius;
}

void CapsuleMesh::_update_lightmap_size() {
	if (get_add_uv2()) {
		// size must have changed, update lightmap size hint
		Size2i _lightmap_size_hint;
		float texel_size = get_lightmap_texel_size();
		float padding = get_uv2_padding();

		float radial_length = radius * Math_PI * 0.5; // circumference of 90 degree bend
		float vertical_length = radial_length * 2 + (height - 2.0 * radius); // total vertical length

		_lightmap_size_hint.x = MAX(1.0, 4.0 * radial_length / texel_size) + padding;
		_lightmap_size_hint.y = MAX(1.0, vertical_length / texel_size) + padding;

		set_lightmap_size_hint(_lightmap_size_hint);
	}
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

void CapsuleMesh::_bind_methods(){
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &CapsuleMesh::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &CapsuleMesh::get_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.001,100.0,0.001,or_greater,suffix:m"), "set_radius", "get_radius");
}

void SphereMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &SphereMesh::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &SphereMesh::get_radius);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &SphereMesh::set_height);
	ClassDB::bind_method(D_METHOD("get_height"), &SphereMesh::get_height);

	ClassDB::bind_method(D_METHOD("set_radial_segments", "radial_segments"), &SphereMesh::set_radial_segments);
	ClassDB::bind_method(D_METHOD("get_radial_segments"), &SphereMesh::get_radial_segments);
	ClassDB::bind_method(D_METHOD("set_rings", "rings"), &SphereMesh::set_rings);
	ClassDB::bind_method(D_METHOD("get_rings"), &SphereMesh::get_rings);

	ClassDB::bind_method(D_METHOD("set_is_hemisphere", "is_hemisphere"), &SphereMesh::set_is_hemisphere);
	ClassDB::bind_method(D_METHOD("get_is_hemisphere"), &SphereMesh::get_is_hemisphere);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.001,100.0,0.001,or_greater,suffix:m"), "set_radius", "get_radius");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_RANGE, "0.001,100.0,0.001,or_greater,suffix:m"), "set_height", "get_height");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "radial_segments", PROPERTY_HINT_RANGE, "1,100,1,or_greater"), "set_radial_segments", "get_radial_segments");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "rings", PROPERTY_HINT_RANGE, "1,100,1,or_greater"), "set_rings", "get_rings");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_hemisphere"), "set_is_hemisphere", "get_is_hemisphere");
}


void SphereMesh::set_radius(const float p_radius) {
	radius = p_radius;
	_update_lightmap_size();
	request_update();
}

float SphereMesh::get_radius() const {
	return radius;
}

void SphereMesh::set_height(const float p_height) {
	height = p_height;
	_update_lightmap_size();
	request_update();
}

float SphereMesh::get_height() const {
	return height;
}

void SphereMesh::set_radial_segments(const int p_radial_segments) {
	radial_segments = p_radial_segments > 4 ? p_radial_segments : 4;
	request_update();
}

int SphereMesh::get_radial_segments() const {
	return radial_segments;
}

void SphereMesh::set_rings(const int p_rings) {
	ERR_FAIL_COND(p_rings < 1);
	rings = p_rings;
	request_update();
}

int SphereMesh::get_rings() const {
	return rings;
}

void SphereMesh::set_is_hemisphere(const bool p_is_hemisphere) {
	is_hemisphere = p_is_hemisphere;
	_update_lightmap_size();
	request_update();
}

bool SphereMesh::get_is_hemisphere() const {
	return is_hemisphere;
}

SphereMesh::SphereMesh() {}

void SphereMesh::_update_lightmap_size() {
	if (get_add_uv2()) {
		// size must have changed, update lightmap size hint
		Size2i _lightmap_size_hint;
		float texel_size = get_lightmap_texel_size();
		float padding = get_uv2_padding();

		float _width = radius * Math_TAU;
		_lightmap_size_hint.x = MAX(1.0, (_width / texel_size) + padding);
		float _height = (is_hemisphere ? 1.0 : 0.5) * height * Math_PI; // note, with hemisphere height is our radius, while with a full sphere it is the diameter..
		_lightmap_size_hint.y = MAX(1.0, (_height / texel_size) + padding);

		set_lightmap_size_hint(_lightmap_size_hint);
	}
}

void SphereMesh::_create_mesh_array(Array &p_arr) const {
	bool _add_uv2 = get_add_uv2();
	float texel_size = get_lightmap_texel_size();
	float _uv2_padding = get_uv2_padding() * texel_size;

	create_mesh_array(p_arr, radius, height, radial_segments, rings, is_hemisphere, _add_uv2, _uv2_padding);
}

void SphereMesh::create_mesh_array(Array &p_arr, float radius, float height, int radial_segments, int rings, bool is_hemisphere, bool p_add_uv2, const float p_uv2_padding) {
	int i, j, prevrow, thisrow, point;
	float x, y, z;

	float scale = height * (is_hemisphere ? 1.0 : 0.5);

	// Only used if we calculate UV2
	float circumference = radius * Math_TAU;
	float horizontal_length = circumference + p_uv2_padding;
	float center_h = 0.5 * circumference / horizontal_length;

	float height_v = scale * Math_PI / ((scale * Math_PI) + p_uv2_padding);

	// set our bounding box

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

	thisrow = 0;
	prevrow = 0;
	for (j = 0; j <= (rings + 1); j++) {
		float v = j;
		float w;

		v /= (rings + 1);
		w = sin(Math_PI * v);
		y = scale * cos(Math_PI * v);

		for (i = 0; i <= radial_segments; i++) {
			float u = i;
			u /= radial_segments;

			x = sin(u * Math_TAU);
			z = cos(u * Math_TAU);

			if (is_hemisphere && y < 0.0) {
				points.push_back(Vector3(x * radius * w, 0.0, z * radius * w));
				normals.push_back(Vector3(0.0, -1.0, 0.0));
			} else {
				Vector3 p = Vector3(x * radius * w, y, z * radius * w);
				points.push_back(p);
				Vector3 normal = Vector3(x * w * scale, radius * (y / scale), z * w * scale);
				normals.push_back(normal.normalized());
			}
			ADD_TANGENT(z, 0.0, -x, 1.0)
			uvs.push_back(Vector2(u, v));
			if (p_add_uv2) {
				float w_h = w * 2.0 * center_h;
				uv2s.push_back(Vector2(center_h + ((u - 0.5) * w_h), v * height_v));
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

/// Box Mesh
/// Box Mesh
/// Box Mesh
void BoxMesh::_update_lightmap_size() {
	if (get_add_uv2()) {
		// size must have changed, update lightmap size hint
		Size2i _lightmap_size_hint;
		float texel_size = get_lightmap_texel_size();
		float padding = get_uv2_padding();

		float width = (size.x + size.z) / texel_size;
		float length = (size.y + size.y + MAX(size.x, size.z)) / texel_size;

		_lightmap_size_hint.x = MAX(1.0, width) + 2.0 * padding;
		_lightmap_size_hint.y = MAX(1.0, length) + 3.0 * padding;

		set_lightmap_size_hint(_lightmap_size_hint);
	}
}

void BoxMesh::_create_mesh_array(Array &p_arr) const {
	// Note about padding, with our box each face of the box faces a different direction so we want a seam
	// around every face. We thus add our padding to the right and bottom of each face.
	// With 3 faces along the width and 2 along the height of the texture we need to adjust our scale
	// accordingly.
	bool _add_uv2 = get_add_uv2();
	float texel_size = get_lightmap_texel_size();
	float _uv2_padding = get_uv2_padding() * texel_size;

	BoxMesh::create_mesh_array(p_arr, size, subdivide_w, subdivide_h, subdivide_d, _add_uv2, _uv2_padding);
}

void BoxMesh::create_mesh_array(Array &p_arr, Vector3 size, int subdivide_w, int subdivide_h, int subdivide_d, bool p_add_uv2, const float p_uv2_padding) {
	int i, j, prevrow, thisrow, point;
	float x, y, z;
	float onethird = 1.0 / 3.0;
	float twothirds = 2.0 / 3.0;

	// Only used if we calculate UV2
	// TODO this could be improved by changing the order depending on which side is the longest (basically the below works best if size.y is the longest)
	float total_h = (size.x + size.z + (2.0 * p_uv2_padding));
	float padding_h = p_uv2_padding / total_h;
	float width_h = size.x / total_h;
	float depth_h = size.z / total_h;
	float total_v = (size.y + size.y + MAX(size.x, size.z) + (3.0 * p_uv2_padding));
	float padding_v = p_uv2_padding / total_v;
	float width_v = size.x / total_v;
	float height_v = size.y / total_v;
	float depth_v = size.z / total_v;

	Vector3 start_pos = size * -0.5;

	// set our bounding box

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

	// front + back
	y = start_pos.y;
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= subdivide_h + 1; j++) {
		float v = j;
		float v2 = v / (subdivide_w + 1.0);
		v /= (2.0 * (subdivide_h + 1.0));

		x = start_pos.x;
		for (i = 0; i <= subdivide_w + 1; i++) {
			float u = i;
			float u2 = u / (subdivide_w + 1.0);
			u /= (3.0 * (subdivide_w + 1.0));

			// front
			points.push_back(Vector3(x, -y, -start_pos.z)); // double negative on the Z!
			normals.push_back(Vector3(0.0, 0.0, 1.0));
			ADD_TANGENT(1.0, 0.0, 0.0, 1.0);
			uvs.push_back(Vector2(u, v));
			if (p_add_uv2) {
				uv2s.push_back(Vector2(u2 * width_h, v2 * height_v));
			}
			point++;

			// back
			points.push_back(Vector3(-x, -y, start_pos.z));
			normals.push_back(Vector3(0.0, 0.0, -1.0));
			ADD_TANGENT(-1.0, 0.0, 0.0, 1.0);
			uvs.push_back(Vector2(twothirds + u, v));
			if (p_add_uv2) {
				uv2s.push_back(Vector2(u2 * width_h, height_v + padding_v + (v2 * height_v)));
			}
			point++;

			if (i > 0 && j > 0) {
				int i2 = i * 2;

				// front
				indices.push_back(prevrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2);
				indices.push_back(thisrow + i2 - 2);

				// back
				indices.push_back(prevrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
			}

			x += size.x / (subdivide_w + 1.0);
		}

		y += size.y / (subdivide_h + 1.0);
		prevrow = thisrow;
		thisrow = point;
	}

	// left + right
	y = start_pos.y;
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= (subdivide_h + 1); j++) {
		float v = j;
		float v2 = v / (subdivide_h + 1.0);
		v /= (2.0 * (subdivide_h + 1.0));

		z = start_pos.z;
		for (i = 0; i <= (subdivide_d + 1); i++) {
			float u = i;
			float u2 = u / (subdivide_d + 1.0);
			u /= (3.0 * (subdivide_d + 1.0));

			// right
			points.push_back(Vector3(-start_pos.x, -y, -z));
			normals.push_back(Vector3(1.0, 0.0, 0.0));
			ADD_TANGENT(0.0, 0.0, -1.0, 1.0);
			uvs.push_back(Vector2(onethird + u, v));
			if (p_add_uv2) {
				uv2s.push_back(Vector2(width_h + padding_h + (u2 * depth_h), v2 * height_v));
			}
			point++;

			// left
			points.push_back(Vector3(start_pos.x, -y, z));
			normals.push_back(Vector3(-1.0, 0.0, 0.0));
			ADD_TANGENT(0.0, 0.0, 1.0, 1.0);
			uvs.push_back(Vector2(u, 0.5 + v));
			if (p_add_uv2) {
				uv2s.push_back(Vector2(width_h + padding_h + (u2 * depth_h), height_v + padding_v + (v2 * height_v)));
			}
			point++;

			if (i > 0 && j > 0) {
				int i2 = i * 2;

				// right
				indices.push_back(prevrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2);
				indices.push_back(thisrow + i2 - 2);

				// left
				indices.push_back(prevrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
			}

			z += size.z / (subdivide_d + 1.0);
		}

		y += size.y / (subdivide_h + 1.0);
		prevrow = thisrow;
		thisrow = point;
	}

	// top + bottom
	z = start_pos.z;
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= (subdivide_d + 1); j++) {
		float v = j;
		float v2 = v / (subdivide_d + 1.0);
		v /= (2.0 * (subdivide_d + 1.0));

		x = start_pos.x;
		for (i = 0; i <= (subdivide_w + 1); i++) {
			float u = i;
			float u2 = u / (subdivide_w + 1.0);
			u /= (3.0 * (subdivide_w + 1.0));

			// top
			points.push_back(Vector3(-x, -start_pos.y, -z));
			normals.push_back(Vector3(0.0, 1.0, 0.0));
			ADD_TANGENT(-1.0, 0.0, 0.0, 1.0);
			uvs.push_back(Vector2(onethird + u, 0.5 + v));
			if (p_add_uv2) {
				uv2s.push_back(Vector2(u2 * width_h, ((height_v + padding_v) * 2.0) + (v2 * depth_v)));
			}
			point++;

			// bottom
			points.push_back(Vector3(x, start_pos.y, -z));
			normals.push_back(Vector3(0.0, -1.0, 0.0));
			ADD_TANGENT(1.0, 0.0, 0.0, 1.0);
			uvs.push_back(Vector2(twothirds + u, 0.5 + v));
			if (p_add_uv2) {
				uv2s.push_back(Vector2(width_h + padding_h + (u2 * depth_h), ((height_v + padding_v) * 2.0) + (v2 * width_v)));
			}
			point++;

			if (i > 0 && j > 0) {
				int i2 = i * 2;

				// top
				indices.push_back(prevrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2);
				indices.push_back(thisrow + i2 - 2);

				// bottom
				indices.push_back(prevrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
			}

			x += size.x / (subdivide_w + 1.0);
		}

		z += size.z / (subdivide_d + 1.0);
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

void BoxMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_size", "size"), &BoxMesh::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &BoxMesh::get_size);

	ClassDB::bind_method(D_METHOD("set_subdivide_width", "subdivide"), &BoxMesh::set_subdivide_width);
	ClassDB::bind_method(D_METHOD("get_subdivide_width"), &BoxMesh::get_subdivide_width);
	ClassDB::bind_method(D_METHOD("set_subdivide_height", "divisions"), &BoxMesh::set_subdivide_height);
	ClassDB::bind_method(D_METHOD("get_subdivide_height"), &BoxMesh::get_subdivide_height);
	ClassDB::bind_method(D_METHOD("set_subdivide_depth", "divisions"), &BoxMesh::set_subdivide_depth);
	ClassDB::bind_method(D_METHOD("get_subdivide_depth"), &BoxMesh::get_subdivide_depth);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "subdivide_width", PROPERTY_HINT_RANGE, "0,100,1,or_greater"), "set_subdivide_width", "get_subdivide_width");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "subdivide_height", PROPERTY_HINT_RANGE, "0,100,1,or_greater"), "set_subdivide_height", "get_subdivide_height");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "subdivide_depth", PROPERTY_HINT_RANGE, "0,100,1,or_greater"), "set_subdivide_depth", "get_subdivide_depth");
}

void BoxMesh::set_size(const Vector3 &p_size) {
	size = p_size;
	_update_lightmap_size();
	request_update();
}

Vector3 BoxMesh::get_size() const {
	return size;
}

void BoxMesh::set_subdivide_width(const int p_divisions) {
	subdivide_w = p_divisions > 0 ? p_divisions : 0;
	request_update();
}

int BoxMesh::get_subdivide_width() const {
	return subdivide_w;
}

void BoxMesh::set_subdivide_height(const int p_divisions) {
	subdivide_h = p_divisions > 0 ? p_divisions : 0;
	request_update();
}

int BoxMesh::get_subdivide_height() const {
	return subdivide_h;
}

void BoxMesh::set_subdivide_depth(const int p_divisions) {
	subdivide_d = p_divisions > 0 ? p_divisions : 0;
	request_update();
}

int BoxMesh::get_subdivide_depth() const {
	return subdivide_d;
}

BoxMesh::BoxMesh() {}
