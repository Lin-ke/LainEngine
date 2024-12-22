#pragma once
#ifndef __MESH_H__
#define __MESH_H__
#include "core/io/resource.h"
#include "core/math/triangle_mesh.h"
#include "function/render/rendering_system/rendering_system.h"
#include "material.h"
namespace lain {

// RS::get_singleton()->material_set_shader(_get_material(), rid);

class Mesh : public Resource {
  LCLASS(Mesh, Resource);

  mutable Ref<TriangleMesh> triangle_mesh;                    //cached
  mutable Vector<Ref<TriangleMesh>> surface_triangle_meshes;  //cached
  mutable Vector<Vector3> debug_lines;
  Size2i lightmap_size_hint;

 public:
  enum PrimitiveType {
    PRIMITIVE_POINTS = RS::PRIMITIVE_POINTS,
    PRIMITIVE_LINES = RS::PRIMITIVE_LINES,
    PRIMITIVE_LINE_STRIP = RS::PRIMITIVE_LINE_STRIP,
    PRIMITIVE_TRIANGLES = RS::PRIMITIVE_TRIANGLES,
    PRIMITIVE_TRIANGLE_STRIP = RS::PRIMITIVE_TRIANGLE_STRIP,
    PRIMITIVE_MAX = RS::PRIMITIVE_MAX,
  };

  enum { NO_INDEX_ARRAY = RS::NO_INDEX_ARRAY, ARRAY_WEIGHTS_SIZE = RS::ARRAY_WEIGHTS_SIZE };
  enum BlendShapeMode {
    BLEND_SHAPE_MODE_NORMALIZED = RS::BLEND_SHAPE_MODE_NORMALIZED,
    BLEND_SHAPE_MODE_RELATIVE = RS::BLEND_SHAPE_MODE_RELATIVE,
  };
  enum ArrayType {
    ARRAY_VERTEX = RS::ARRAY_VERTEX,
    ARRAY_NORMAL = RS::ARRAY_NORMAL,
    ARRAY_TANGENT = RS::ARRAY_TANGENT,
    ARRAY_COLOR = RS::ARRAY_COLOR,
    ARRAY_TEX_UV = RS::ARRAY_TEX_UV,
    ARRAY_TEX_UV2 = RS::ARRAY_TEX_UV2,
    ARRAY_CUSTOM0 = RS::ARRAY_CUSTOM0,
    ARRAY_CUSTOM1 = RS::ARRAY_CUSTOM1,
    ARRAY_CUSTOM2 = RS::ARRAY_CUSTOM2,
    ARRAY_CUSTOM3 = RS::ARRAY_CUSTOM3,
    ARRAY_BONES = RS::ARRAY_BONES,
    ARRAY_WEIGHTS = RS::ARRAY_WEIGHTS,
    ARRAY_INDEX = RS::ARRAY_INDEX,
    ARRAY_MAX = RS::ARRAY_MAX

  };

  enum ArrayCustomFormat {
    ARRAY_CUSTOM_RGBA8_UNORM,
    ARRAY_CUSTOM_RGBA8_SNORM,
    ARRAY_CUSTOM_RG_HALF,
    ARRAY_CUSTOM_RGBA_HALF,
    ARRAY_CUSTOM_R_FLOAT,
    ARRAY_CUSTOM_RG_FLOAT,
    ARRAY_CUSTOM_RGB_FLOAT,
    ARRAY_CUSTOM_RGBA_FLOAT,
    ARRAY_CUSTOM_MAX
  };

  enum ArrayFormat : uint64_t {
    ARRAY_FORMAT_VERTEX = RS::ARRAY_FORMAT_VERTEX,
    ARRAY_FORMAT_NORMAL = RS::ARRAY_FORMAT_NORMAL,
    ARRAY_FORMAT_TANGENT = RS::ARRAY_FORMAT_TANGENT,
    ARRAY_FORMAT_COLOR = RS::ARRAY_FORMAT_COLOR,
    ARRAY_FORMAT_TEX_UV = RS::ARRAY_FORMAT_TEX_UV,
    ARRAY_FORMAT_TEX_UV2 = RS::ARRAY_FORMAT_TEX_UV2,
    ARRAY_FORMAT_CUSTOM0 = RS::ARRAY_FORMAT_CUSTOM0,
    ARRAY_FORMAT_CUSTOM1 = RS::ARRAY_FORMAT_CUSTOM1,
    ARRAY_FORMAT_CUSTOM2 = RS::ARRAY_FORMAT_CUSTOM2,
    ARRAY_FORMAT_CUSTOM3 = RS::ARRAY_FORMAT_CUSTOM3,
    ARRAY_FORMAT_BONES = RS::ARRAY_FORMAT_BONES,
    ARRAY_FORMAT_WEIGHTS = RS::ARRAY_FORMAT_WEIGHTS,
    ARRAY_FORMAT_INDEX = RS::ARRAY_FORMAT_INDEX,

    ARRAY_FORMAT_BLEND_SHAPE_MASK = RS::ARRAY_FORMAT_BLEND_SHAPE_MASK,

    ARRAY_FORMAT_CUSTOM_BASE = RS::ARRAY_FORMAT_CUSTOM_BASE,
    ARRAY_FORMAT_CUSTOM_BITS = RS::ARRAY_FORMAT_CUSTOM_BITS,
    ARRAY_FORMAT_CUSTOM0_SHIFT = RS::ARRAY_FORMAT_CUSTOM0_SHIFT,
    ARRAY_FORMAT_CUSTOM1_SHIFT = RS::ARRAY_FORMAT_CUSTOM1_SHIFT,
    ARRAY_FORMAT_CUSTOM2_SHIFT = RS::ARRAY_FORMAT_CUSTOM2_SHIFT,
    ARRAY_FORMAT_CUSTOM3_SHIFT = RS::ARRAY_FORMAT_CUSTOM3_SHIFT,

    ARRAY_FORMAT_CUSTOM_MASK = RS::ARRAY_FORMAT_CUSTOM_MASK,
    ARRAY_COMPRESS_FLAGS_BASE = RS::ARRAY_COMPRESS_FLAGS_BASE,

    ARRAY_FLAG_USE_2D_VERTICES = RS::ARRAY_FLAG_USE_2D_VERTICES,
    ARRAY_FLAG_USE_DYNAMIC_UPDATE = RS::ARRAY_FLAG_USE_DYNAMIC_UPDATE,
    ARRAY_FLAG_USE_8_BONE_WEIGHTS = RS::ARRAY_FLAG_USE_8_BONE_WEIGHTS,

    ARRAY_FLAG_USES_EMPTY_VERTEX_ARRAY = RS::ARRAY_FLAG_USES_EMPTY_VERTEX_ARRAY,
    ARRAY_FLAG_COMPRESS_ATTRIBUTES = RS::ARRAY_FLAG_COMPRESS_ATTRIBUTES,

    ARRAY_FLAG_FORMAT_VERSION_BASE = RS::ARRAY_FLAG_FORMAT_VERSION_BASE,
    ARRAY_FLAG_FORMAT_VERSION_SHIFT = RS::ARRAY_FLAG_FORMAT_VERSION_SHIFT,
    ARRAY_FLAG_FORMAT_VERSION_1 = RS::ARRAY_FLAG_FORMAT_VERSION_1,
    ARRAY_FLAG_FORMAT_VERSION_2 = (uint64_t)RS::ARRAY_FLAG_FORMAT_VERSION_2,
    ARRAY_FLAG_FORMAT_CURRENT_VERSION = (uint64_t)RS::ARRAY_FLAG_FORMAT_CURRENT_VERSION,
    ARRAY_FLAG_FORMAT_VERSION_MASK = RS::ARRAY_FLAG_FORMAT_VERSION_MASK,
  };
  virtual int get_surface_count() const;
  virtual int surface_get_array_len(int p_idx) const;
  virtual int surface_get_array_index_len(int p_idx) const;
  virtual Array surface_get_arrays(int p_surface) const;
  virtual Array surface_get_blend_shape_arrays(int p_surface) const;
  virtual Dictionary surface_get_lods(int p_surface) const;
  virtual BitField<ArrayFormat> surface_get_format(int p_idx) const;
  virtual PrimitiveType surface_get_primitive_type(int p_idx) const;
  virtual void surface_set_material(int p_idx, const Ref<Material>& p_material);
  virtual Ref<Material> surface_get_material(int p_idx) const;
  virtual int get_blend_shape_count() const;
  virtual StringName get_blend_shape_name(int p_index) const;
  virtual void set_blend_shape_name(int p_index, const StringName& p_name);
  virtual AABB get_aabb() const;

  Vector<Face3> get_faces() const;
  Vector<Face3> get_surface_faces(int p_surface) const;
  Ref<TriangleMesh> generate_triangle_mesh() const;
  Ref<TriangleMesh> generate_surface_triangle_mesh(int p_surface) const;
  void generate_debug_mesh_lines(Vector<Vector3>& r_lines);
  void generate_debug_mesh_indices(Vector<Vector3>& r_points);

  Ref<Mesh> create_outline(float p_margin) const;

  void set_lightmap_size_hint(const Size2i& p_size);
  Size2i get_lightmap_size_hint() const;
  void clear_cache() const;

  Mesh() {}
};

class ArrayMesh : public Mesh {
  LCLASS(ArrayMesh, Mesh);
  RES_BASE_EXTENSION("mesh");

  PackedStringArray _get_blend_shape_names() const;
  void _set_blend_shape_names(const PackedStringArray& p_names);

  Array _get_surfaces() const;
  void _set_surfaces(const Array& p_data);
  Ref<ArrayMesh> shadow_mesh;

 private:
  struct Surface {
    uint64_t format = 0;
    int array_length = 0;
    int index_array_length = 0;
    PrimitiveType primitive = PrimitiveType::PRIMITIVE_MAX;

    String name;
    AABB aabb;
    Ref<Material> material;
    bool is_2d = false;
  };
  Vector<Surface> surfaces;
  mutable RID mesh;
  AABB aabb;
  BlendShapeMode blend_shape_mode = BLEND_SHAPE_MODE_RELATIVE;
  Vector<StringName> blend_shapes;
  AABB custom_aabb;

  _FORCE_INLINE_ void _create_if_empty() const;
  void _recompute_aabb();

 protected:
  virtual bool _is_generated() const { return false; }

  bool _set(const StringName& p_name, const Variant& p_value);
  bool _get(const StringName& p_name, Variant& r_ret) const;
  void _get_property_list(List<PropertyInfo>* p_list) const;
  bool surface_index_0 = false;

  virtual void reset_state() override;

  static void _bind_methods();

 public:
  void add_surface_from_arrays(PrimitiveType p_primitive, const Array& p_arrays, const Array& p_blend_shapes = TypedArray<Array>(), const Dictionary& p_lods = Dictionary(),
                               BitField<ArrayFormat> p_flags = 0);

  void add_surface(BitField<ArrayFormat> p_format, PrimitiveType p_primitive, const Vector<uint8_t>& p_array, const Vector<uint8_t>& p_attribute_array,
                   const Vector<uint8_t>& p_skin_array, int p_vertex_count, const Vector<uint8_t>& p_index_array, int p_index_count, const AABB& p_aabb,
                   const Vector<uint8_t>& p_blend_shape_data = Vector<uint8_t>(), const Vector<AABB>& p_bone_aabbs = Vector<AABB>(),
                   const Vector<RS::SurfaceData::LOD>& p_lods = Vector<RS::SurfaceData::LOD>(), const Vector4 p_uv_scale = Vector4());

  Array surface_get_arrays(int p_surface) const override;
  Array surface_get_blend_shape_arrays(int p_surface) const override;
  Dictionary surface_get_lods(int p_surface) const override;

  void add_blend_shape(const StringName& p_name);
  int get_blend_shape_count() const override;
  StringName get_blend_shape_name(int p_index) const override;
  void set_blend_shape_name(int p_index, const StringName& p_name) override;
  void clear_blend_shapes();

  void set_blend_shape_mode(BlendShapeMode p_mode);
  BlendShapeMode get_blend_shape_mode() const;

  void surface_update_vertex_region(int p_surface, int p_offset, const Vector<uint8_t>& p_data);
  void surface_update_attribute_region(int p_surface, int p_offset, const Vector<uint8_t>& p_data);
  void surface_update_skin_region(int p_surface, int p_offset, const Vector<uint8_t>& p_data);

  int get_surface_count() const override;

  void clear_surfaces();

  void surface_set_custom_aabb(int p_idx, const AABB& p_aabb);  //only recognized by driver

  int surface_get_array_len(int p_idx) const override;
  int surface_get_array_index_len(int p_idx) const override;
  BitField<ArrayFormat> surface_get_format(int p_idx) const override;
  PrimitiveType surface_get_primitive_type(int p_idx) const override;

  virtual void surface_set_material(int p_idx, const Ref<Material>& p_material) override;
  virtual Ref<Material> surface_get_material(int p_idx) const override;

  int surface_find_by_name(const String& p_name) const;
  void surface_set_name(int p_idx, const String& p_name);
  String surface_get_name(int p_idx) const;

  void set_custom_aabb(const AABB& p_custom);
  AABB get_custom_aabb() const;

  AABB get_aabb() const override;
  virtual RID GetRID() const override;

  void regen_normal_maps();

  Error lightmap_unwrap(const Transform3D& p_base_transform = Transform3D(), float p_texel_size = 0.05);
  Error lightmap_unwrap_cached(const Transform3D& p_base_transform, float p_texel_size, const Vector<uint8_t>& p_src_cache, Vector<uint8_t>& r_dst_cache,
                               bool p_generate_cache = true);

  virtual void reload_from_file() override;

  void set_shadow_mesh(const Ref<ArrayMesh>& p_mesh);
  Ref<ArrayMesh> get_shadow_mesh() const;

  ArrayMesh();

  ~ArrayMesh();
};
VARIANT_ENUM_CAST(Mesh::ArrayType);
VARIANT_BITFIELD_CAST(Mesh::ArrayFormat);
VARIANT_ENUM_CAST(Mesh::ArrayCustomFormat);
VARIANT_ENUM_CAST(Mesh::PrimitiveType);
VARIANT_ENUM_CAST(Mesh::BlendShapeMode);
}  // namespace lain

#endif