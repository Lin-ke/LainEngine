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
        ERR_CONTINUE_MSG((len % 3) != 0, vformat("Ignoring surface %d, incorrect %s count: %d (for PRIMITIVE_TRIANGLES).", i,
                                                 (surface_get_format(i) & ARRAY_FORMAT_INDEX) ? "index" : "vertex", len));
        faces_size += len;
      } break;
      case PRIMITIVE_TRIANGLE_STRIP: {
        int len = (surface_get_format(i) & ARRAY_FORMAT_INDEX) ? surface_get_array_index_len(i) : surface_get_array_len(i);
        // Don't error if zero, it's valid (we'll just skip it later).
        ERR_CONTINUE_MSG(len != 0 && len < 3, vformat("Ignoring surface %d, incorrect %s count: %d (for PRIMITIVE_TRIANGLE_STRIP).", i,
                                                      (surface_get_format(i) & ARRAY_FORMAT_INDEX) ? "index" : "vertex", len));
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
  Vector3* facesw = faces.ptrw();
  int32_t* surface_indicesw = surface_indices.ptrw();

  int widx = 0;

  for (int i = 0; i < get_surface_count(); i++) {
    Mesh::PrimitiveType primitive = surface_get_primitive_type(i);
    if (primitive != PRIMITIVE_TRIANGLES && primitive != PRIMITIVE_TRIANGLE_STRIP) {
      continue;
    }
    int len = (surface_get_format(i) & ARRAY_FORMAT_INDEX) ? surface_get_array_index_len(i) : surface_get_array_len(i);
    if ((primitive == PRIMITIVE_TRIANGLES && (len == 0 || (len % 3) != 0)) || (primitive == PRIMITIVE_TRIANGLE_STRIP && len < 3) ||
        (surface_get_format(i) & ARRAY_FLAG_USES_EMPTY_VERTEX_ARRAY)) {
      // Error was already shown, just skip (including zero).
      continue;
    }

    Array a = surface_get_arrays(i);
    ERR_FAIL_COND_V(a.is_empty(), Ref<TriangleMesh>());

    int vc = surface_get_array_len(i);
    Vector<Vector3> vertices = a[ARRAY_VERTEX];
    ERR_FAIL_COND_V(vertices.is_empty(), Ref<TriangleMesh>());
    const Vector3* vr = vertices.ptr();

    int32_t from_index = widx / 3;

    if (surface_get_format(i) & ARRAY_FORMAT_INDEX) {
      int ic = surface_get_array_index_len(i);
      Vector<int> indices = a[ARRAY_INDEX];
      const int* ir = indices.ptr();

      if (primitive == PRIMITIVE_TRIANGLES) {
        for (int j = 0; j < ic; j++) {
          int index = ir[j];
          ERR_FAIL_COND_V(index >= vc, Ref<TriangleMesh>());
          facesw[widx++] = vr[index];
        }
      } else {  // PRIMITIVE_TRIANGLE_STRIP
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
      } else {  // PRIMITIVE_TRIANGLE_STRIP
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
  Vector3* facesw = faces.ptrw();

  Array a = surface_get_arrays(p_surface);
  ERR_FAIL_COND_V(a.is_empty(), Ref<TriangleMesh>());

  int vc = surface_get_array_len(p_surface);
  Vector<Vector3> vertices = a[ARRAY_VERTEX];
  const Vector3* vr = vertices.ptr();
  int widx = 0;

  if (surface_get_format(p_surface) & ARRAY_FORMAT_INDEX) {
    int ic = surface_get_array_index_len(p_surface);
    Vector<int> indices = a[ARRAY_INDEX];
    const int* ir = indices.ptr();

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

  debug_lines.resize(tm->get_triangles().size() * 6);  // 3 lines x 2 points each line

  const int* ind_r = triangle_indices.ptr();
  const Vector3* ver_r = vertices.ptr();
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

Array lain::Mesh::surface_get_blend_shape_arrays(int p_surface) const {
  return Array();
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

void Mesh::generate_debug_mesh_indices(Vector<Vector3>& r_points) {
  Ref<TriangleMesh> tm = generate_triangle_mesh();
  if (tm.is_null()) {
    return;
  }

  Vector<Vector3> vertices = tm->get_vertices();

  int vertices_size = vertices.size();
  r_points.resize(vertices_size);
  for (int i = 0; i < vertices_size; ++i) {
    r_points.write[i] = vertices[i];  // @opt
  }
}
void Mesh::set_lightmap_size_hint(const Size2i& p_size) {
  lightmap_size_hint = p_size;
}

Size2i lain::Mesh::get_lightmap_size_hint() const {
  return lightmap_size_hint;
}

void Mesh::clear_cache() const {
  triangle_mesh.unref();
  debug_lines.clear();
}

void lain::ArrayMesh::reload_from_file() {}

lain::ArrayMesh::ArrayMesh() {}
ArrayMesh::~ArrayMesh() {
  if (mesh.is_valid()) {
    ERR_FAIL_NULL(RS::get_singleton());
    RS::get_singleton()->free(mesh);
  }
}

void lain::ArrayMesh::_recompute_aabb() {
  // regenerate AABB
  aabb = AABB();

  for (int i = 0; i < surfaces.size(); i++) {
    if (i == 0) {
      aabb = surfaces[i].aabb;
    } else {
      aabb.merge_with(surfaces[i].aabb);
    }
  }
}

bool ArrayMesh::_set(const StringName& p_name, const Variant& p_value) {
  String sname = p_name;

  if (sname.begins_with("surface_")) {
    int sl = sname.find("/");
    if (sl == -1) {
      return false;
    }
    int idx = sname.substr(8, sl - 8).to_int();

    String what = sname.get_slicec('/', 1);
    if (what == "material") {
      surface_set_material(idx, p_value);
    } else if (what == "name") {
      surface_set_name(idx, p_value);
    }
    return true;
  }
  return false;
}
bool ArrayMesh::_get(const StringName& p_name, Variant& r_ret) const {
  if (_is_generated()) {
    return false;
  }

  String sname = p_name;
  if (sname.begins_with("surface_")) {
    int sl = sname.find("/");
    if (sl == -1) {
      return false;
    }
    int idx = sname.substr(8, sl - 8).to_int();
    String what = sname.get_slicec('/', 1);
    if (what == "material") {
      r_ret = surface_get_material(idx);
    } else if (what == "name") {
      r_ret = surface_get_name(idx);
    }
    return true;
  }

  return false;
}
void ArrayMesh::_get_property_list(List<PropertyInfo>* p_list) const {
  if (_is_generated()) {
    return;
  }

  for (int i = 0; i < surfaces.size(); i++) {
    p_list->push_back(PropertyInfo(Variant::STRING, "surface_" + itos(i) + "/name", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR));
    if (surfaces[i].is_2d) {
      p_list->push_back(
          PropertyInfo(Variant::OBJECT, "surface_" + itos(i) + "/material", PROPERTY_HINT_RESOURCE_TYPE, "CanvasItemMaterial,ShaderMaterial", PROPERTY_USAGE_EDITOR));
    } else {
      p_list->push_back(
          PropertyInfo(Variant::OBJECT, "surface_" + itos(i) + "/material", PROPERTY_HINT_RESOURCE_TYPE, "BaseMaterial3D,ShaderMaterial", PROPERTY_USAGE_EDITOR));
    }
  }
}
void lain::ArrayMesh::reset_state() {}

void lain::ArrayMesh::_bind_methods() {
  ClassDB::bind_method(D_METHOD("_set_surfaces", "surfaces"), &ArrayMesh::_set_surfaces);
  ClassDB::bind_method(D_METHOD("_get_surfaces"), &ArrayMesh::_get_surfaces);

  ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "_surfaces", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL), "_set_surfaces", "_get_surfaces");
}

void lain::ArrayMesh::add_surface_from_arrays(PrimitiveType p_primitive, const Array& p_arrays, const Array& p_blend_shapes, const Dictionary& p_lods,
                                              BitField<ArrayFormat> p_flags) {
  ERR_FAIL_COND(p_blend_shapes.size() != blend_shapes.size());
  ERR_FAIL_COND(p_arrays.size() != ARRAY_MAX);

  RS::SurfaceData surface;

  Error err = RS::get_singleton()->mesh_create_surface_data_from_arrays(&surface, (RS::PrimitiveType)p_primitive, p_arrays, p_blend_shapes, p_lods, p_flags);
  ERR_FAIL_COND(err != OK);
  add_surface(surface.format, PrimitiveType(surface.primitive), surface.vertex_data, surface.attribute_data, surface.skin_data, surface.vertex_count, surface.index_data,
              surface.index_count, surface.aabb, surface.blend_shape_data, surface.bone_aabbs, surface.lods, surface.uv_scale);
}

void lain::ArrayMesh::add_surface(BitField<ArrayFormat> p_format, PrimitiveType p_primitive, const Vector<uint8_t>& p_array, const Vector<uint8_t>& p_attribute_array,
                                  const Vector<uint8_t>& p_skin_array, int p_vertex_count, const Vector<uint8_t>& p_index_array, int p_index_count, const AABB& p_aabb,
                                  const Vector<uint8_t>& p_blend_shape_data, const Vector<AABB>& p_bone_aabbs, const Vector<RS::SurfaceData::LOD>& p_lods,
                                  const Vector4 p_uv_scale) {
  ERR_FAIL_COND(surfaces.size() == RS::MAX_MESH_SURFACES);
  _create_if_empty();

  Surface s;
  s.aabb = p_aabb;
  s.is_2d = p_format & ARRAY_FLAG_USE_2D_VERTICES;
  s.primitive = p_primitive;
  s.array_length = p_vertex_count;
  s.index_array_length = p_index_count;
  s.format = p_format;

  surfaces.push_back(s);
  _recompute_aabb();

  RS::SurfaceData sd;
  sd.format = p_format;
  sd.primitive = RS::PrimitiveType(p_primitive);
  sd.aabb = p_aabb;
  sd.vertex_count = p_vertex_count;
  sd.vertex_data = p_array;
  sd.attribute_data = p_attribute_array;
  sd.skin_data = p_skin_array;
  sd.index_count = p_index_count;
  sd.index_data = p_index_array;
  sd.blend_shape_data = p_blend_shape_data;
  sd.bone_aabbs = p_bone_aabbs;
  sd.lods = p_lods;
  sd.uv_scale = p_uv_scale;

  RS::get_singleton()->mesh_add_surface(mesh, sd);

  clear_cache();
  notify_property_list_changed();
  emit_changed();
}

void ArrayMesh::_set_blend_shape_names(const PackedStringArray& p_names) {
  ERR_FAIL_COND(surfaces.size() > 0);

  blend_shapes.resize(p_names.size());
  for (int i = 0; i < p_names.size(); i++) {
    blend_shapes.write[i] = p_names[i];
  }

  if (mesh.is_valid()) {
    RS::get_singleton()->mesh_set_blend_shape_count(mesh, blend_shapes.size());
  }
}
void ArrayMesh::set_blend_shape_name(int p_index, const StringName& p_name) {
  ERR_FAIL_INDEX(p_index, blend_shapes.size());

  StringName shape_name = p_name;
  int found = blend_shapes.find(shape_name);
  if (found != -1 && found != p_index) {
    int count = 2;
    do {
      shape_name = String(p_name) + " " + itos(count);
      count++;
    } while (blend_shapes.has(shape_name));
  }

  blend_shapes.write[p_index] = shape_name;
}
ArrayMesh::PrimitiveType ArrayMesh::surface_get_primitive_type(int p_idx) const {
  ERR_FAIL_INDEX_V(p_idx, surfaces.size(), PRIMITIVE_LINES);
  return surfaces[p_idx].primitive;
}
void ArrayMesh::surface_set_material(int p_idx, const Ref<Material>& p_material) {
  ERR_FAIL_INDEX(p_idx, surfaces.size());
  if (surfaces[p_idx].material == p_material) {
    return;
  }
  surfaces.write[p_idx].material = p_material;
  RS::get_singleton()->mesh_surface_set_material(mesh, p_idx, p_material.is_null() ? RID() : p_material->GetRID());

  emit_changed();
}

void ArrayMesh::surface_set_name(int p_idx, const String& p_name) {
  ERR_FAIL_INDEX(p_idx, surfaces.size());

  surfaces.write[p_idx].name = p_name;
  emit_changed();
}

int ArrayMesh::surface_get_array_len(int p_idx) const {
  ERR_FAIL_INDEX_V(p_idx, surfaces.size(), -1);
  return surfaces[p_idx].array_length;
}
Ref<Material> ArrayMesh::surface_get_material(int p_idx) const {
  ERR_FAIL_INDEX_V(p_idx, surfaces.size(), Ref<Material>());
  return surfaces[p_idx].material;
}

Array ArrayMesh::_get_surfaces() const {
  if (mesh.is_null()) {
    return Array();
  }

  Array ret;
  for (int i = 0; i < surfaces.size(); i++) {
    RS::SurfaceData surface = RS::get_singleton()->mesh_get_surface(mesh, i);
    Dictionary data;
    data["format"] = surface.format;
    data["primitive"] = surface.primitive;
    data["vertex_data"] = surface.vertex_data;
    data["vertex_count"] = surface.vertex_count;
    if (surface.skin_data.size()) {
      data["skin_data"] = surface.skin_data;
    }
    if (surface.attribute_data.size()) {
      data["attribute_data"] = surface.attribute_data;
    }
    data["aabb"] = surface.aabb;
    data["uv_scale"] = surface.uv_scale;
    if (surface.index_count) {
      data["index_data"] = surface.index_data;
      data["index_count"] = surface.index_count;
    };

    Array lods;
    for (int j = 0; j < surface.lods.size(); j++) {
      lods.push_back(surface.lods[j].edge_length);
      lods.push_back(surface.lods[j].index_data);
    }

    if (lods.size()) {
      data["lods"] = lods;
    }

    Array bone_aabbs;
    for (int j = 0; j < surface.bone_aabbs.size(); j++) {
      bone_aabbs.push_back(surface.bone_aabbs[j]);
    }
    if (bone_aabbs.size()) {
      data["bone_aabbs"] = bone_aabbs;
    }

    if (surface.blend_shape_data.size()) {
      data["blend_shapes"] = surface.blend_shape_data;
    }

    if (surfaces[i].material.is_valid()) {
      data["material"] = surfaces[i].material;
    }

    if (!surfaces[i].name.is_empty()) {
      data["name"] = surfaces[i].name;
    }

    if (surfaces[i].is_2d) {
      data["2d"] = true;
    }

    ret.push_back(data);
  }

  return ret;
}

void ArrayMesh::_set_surfaces(const Array& p_surfaces) {
  Vector<RS::SurfaceData> surface_data;
  Vector<Ref<Material>> surface_materials;
  Vector<String> surface_names;
  Vector<bool> surface_2d;

  for (int i = 0; i < p_surfaces.size(); i++) {
    RS::SurfaceData surface;
    Dictionary d = p_surfaces[i];
    ERR_FAIL_COND(!d.has("format"));
    ERR_FAIL_COND(!d.has("primitive"));
    ERR_FAIL_COND(!d.has("vertex_data"));
    ERR_FAIL_COND(!d.has("vertex_count"));
    ERR_FAIL_COND(!d.has("aabb"));
    surface.format = d["format"];
    surface.primitive = RS::PrimitiveType(int(d["primitive"]));
    surface.vertex_data = d["vertex_data"];
    surface.vertex_count = d["vertex_count"];
    if (d.has("attribute_data")) {
      surface.attribute_data = d["attribute_data"];
    }
    if (d.has("skin_data")) {
      surface.skin_data = d["skin_data"];
    }
    surface.aabb = d["aabb"];

    if (d.has("uv_scale")) {
      surface.uv_scale = d["uv_scale"];
    }

    if (d.has("index_data")) {
      ERR_FAIL_COND(!d.has("index_count"));
      surface.index_data = d["index_data"];
      surface.index_count = d["index_count"];
    }

    if (d.has("lods")) {
      Array lods = d["lods"];
      ERR_FAIL_COND(lods.size() & 1);  //must be even
      for (int j = 0; j < lods.size(); j += 2) {
        RS::SurfaceData::LOD lod;
        lod.edge_length = lods[j + 0];
        lod.index_data = lods[j + 1];
        surface.lods.push_back(lod);
      }
    }

    if (d.has("bone_aabbs")) {
      Array bone_aabbs = d["bone_aabbs"];
      for (int j = 0; j < bone_aabbs.size(); j++) {
        surface.bone_aabbs.push_back(bone_aabbs[j]);
      }
    }

    if (d.has("blend_shapes")) {
      surface.blend_shape_data = d["blend_shapes"];
    }

    Ref<Material> material;
    if (d.has("material")) {
      material = d["material"];
      if (material.is_valid()) {
        surface.material = material->GetRID();
      }
    }

    String surf_name;
    if (d.has("name")) {
      surf_name = d["name"];
    }

    bool _2d = false;
    if (d.has("2d")) {
      _2d = d["2d"];
    }

    surface_data.push_back(surface);
    surface_materials.push_back(material);
    surface_names.push_back(surf_name);
    surface_2d.push_back(_2d);
  }

  if (mesh.is_valid()) {
    //if mesh exists, it needs to be updated
    RS::get_singleton()->mesh_clear(mesh);
    for (int i = 0; i < surface_data.size(); i++) {
      RS::get_singleton()->mesh_add_surface(mesh, surface_data[i]);
    }
  } else {
    // if mesh does not exist (first time this is loaded, most likely),
    // we can create it with a single call, which is a lot more efficient and thread friendly
    mesh = RS::get_singleton()->mesh_create_from_surfaces(surface_data, blend_shapes.size());
    RS::get_singleton()->mesh_set_blend_shape_mode(mesh, (RS::BlendShapeMode)blend_shape_mode);
    RS::get_singleton()->mesh_set_path(mesh, GetPath());
  }

  surfaces.clear();

  aabb = AABB();
  for (int i = 0; i < surface_data.size(); i++) {
    Surface s;
    s.aabb = surface_data[i].aabb;
    if (i == 0) {
      aabb = s.aabb;
    } else {
      aabb.merge_with(s.aabb);
    }

    s.material = surface_materials[i];
    s.is_2d = surface_2d[i];
    s.name = surface_names[i];

    s.format = surface_data[i].format;
    s.primitive = PrimitiveType(surface_data[i].primitive);
    s.array_length = surface_data[i].vertex_count;
    s.index_array_length = surface_data[i].index_count;

    surfaces.push_back(s);
  }
}

Array ArrayMesh::surface_get_arrays(int p_surface) const {
  ERR_FAIL_INDEX_V(p_surface, surfaces.size(), Array());
  return RS::get_singleton()->mesh_surface_get_arrays(mesh, p_surface);
}

Array lain::ArrayMesh::surface_get_blend_shape_arrays(int p_surface) const {
  return Array();
}

Dictionary lain::ArrayMesh::surface_get_lods(int p_surface) const {
  return RS::get_singleton()->mesh_surface_get_lods(mesh, p_surface);
}

int lain::ArrayMesh::get_blend_shape_count() const {
  return 0;
}

StringName lain::ArrayMesh::get_blend_shape_name(int p_index) const {
  return StringName();
}

int ArrayMesh::get_surface_count() const {
  return surfaces.size();
}

BitField<Mesh::ArrayFormat> ArrayMesh::surface_get_format(int p_idx) const {
  ERR_FAIL_INDEX_V(p_idx, surfaces.size(), 0);
  return surfaces[p_idx].format;
}
int ArrayMesh::surface_get_array_index_len(int p_idx) const {
  ERR_FAIL_INDEX_V(p_idx, surfaces.size(), -1);
  return surfaces[p_idx].index_array_length;
}

AABB lain::ArrayMesh::get_aabb() const {
  return aabb;
}

String ArrayMesh::surface_get_name(int p_idx) const {
  ERR_FAIL_INDEX_V(p_idx, surfaces.size(), String());
  return surfaces[p_idx].name;
}

RID lain::ArrayMesh::GetRID() const {
  if (!mesh.is_valid()) {
    mesh = RS::get_singleton()->mesh_create();
    RS::get_singleton()->mesh_set_blend_shape_mode(mesh, (RS::BlendShapeMode)blend_shape_mode);
    RS::get_singleton()->mesh_set_blend_shape_count(mesh, blend_shapes.size());
    RS::get_singleton()->mesh_set_path(mesh, GetPath());
  }
  return mesh;
}


void ArrayMesh::add_blend_shape(const StringName &p_name) {
	ERR_FAIL_COND_MSG(surfaces.size(), "Can't add a shape key count if surfaces are already created.");

	StringName shape_name = p_name;

	if (blend_shapes.has(shape_name)) {
		int count = 2;
		do {
			shape_name = String(p_name) + " " + itos(count);
			count++;
		} while (blend_shapes.has(shape_name));
	}

	blend_shapes.push_back(shape_name);

	if (mesh.is_valid()) {
		RS::get_singleton()->mesh_set_blend_shape_count(mesh, blend_shapes.size());
	}
}

void ArrayMesh::set_shadow_mesh(const Ref<ArrayMesh> &p_mesh) {
	// shadow_mesh = p_mesh;
	// if (shadow_mesh.is_valid()) {
	// 	RS::get_singleton()->mesh_set_shadow_mesh(mesh, shadow_mesh->GetRID());
	// } else {
	// 	RS::get_singleton()->mesh_set_shadow_mesh(mesh, RID());
	// }
}

void ArrayMesh::_create_if_empty() const {
	if (!mesh.is_valid()) {
		mesh = RS::get_singleton()->mesh_create();
		RS::get_singleton()->mesh_set_blend_shape_mode(mesh, (RS::BlendShapeMode)blend_shape_mode);
		RS::get_singleton()->mesh_set_blend_shape_count(mesh, blend_shapes.size());
		RS::get_singleton()->mesh_set_path(mesh, GetPath());
	}
}

void ArrayMesh::set_blend_shape_mode(BlendShapeMode p_mode) {
	blend_shape_mode = p_mode;
	if (mesh.is_valid()) {
		RS::get_singleton()->mesh_set_blend_shape_mode(mesh, (RS::BlendShapeMode)p_mode);
	}
}
