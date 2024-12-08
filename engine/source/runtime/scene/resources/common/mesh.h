#pragma once
#ifndef __MESH_H__
#define __MESH_H__
#include "core/io/resource.h"
#include "function/render/rendering_system/rendering_system.h"
#include "core/math/triangle_mesh.h"
#include "material.h"
namespace lain {

	// RS::get_singleton()->material_set_shader(_get_material(), rid);

class Mesh : public Resource {
	LCLASS(Mesh, Resource);
	

	mutable Ref<TriangleMesh> triangle_mesh; //cached
	mutable Vector<Ref<TriangleMesh>> surface_triangle_meshes; //cached 
public:
	enum PrimitiveType {
		PRIMITIVE_POINTS = RS::PRIMITIVE_POINTS,
		PRIMITIVE_LINES = RS::PRIMITIVE_LINES,
		PRIMITIVE_LINE_STRIP = RS::PRIMITIVE_LINE_STRIP,
		PRIMITIVE_TRIANGLES = RS::PRIMITIVE_TRIANGLES,
		PRIMITIVE_TRIANGLE_STRIP = RS::PRIMITIVE_TRIANGLE_STRIP,
		PRIMITIVE_MAX = RS::PRIMITIVE_MAX,
	};

		enum {
		NO_INDEX_ARRAY = RS::NO_INDEX_ARRAY,
		ARRAY_WEIGHTS_SIZE = RS::ARRAY_WEIGHTS_SIZE
	};
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
	virtual Vector<Variant> surface_get_blend_shape_arrays(int p_surface) const;
	virtual Dictionary surface_get_lods(int p_surface) const;
	virtual BitField<ArrayFormat> surface_get_format(int p_idx) const;
	virtual PrimitiveType surface_get_primitive_type(int p_idx) const;
	virtual void surface_set_material(int p_idx, const Ref<Material> &p_material);
	virtual Ref<Material> surface_get_material(int p_idx) const;
	virtual int get_blend_shape_count() const;
	virtual StringName get_blend_shape_name(int p_index) const;
	virtual void set_blend_shape_name(int p_index, const StringName &p_name);
	virtual AABB get_aabb() const;

	Vector<Face3> get_faces() const;
	Vector<Face3> get_surface_faces(int p_surface) const;
	Ref<TriangleMesh> generate_triangle_mesh() const;
	Ref<TriangleMesh> generate_surface_triangle_mesh(int p_surface) const;
	void generate_debug_mesh_lines(Vector<Vector3> &r_lines);
	void generate_debug_mesh_indices(Vector<Vector3> &r_points);

	Ref<Mesh> create_outline(float p_margin) const;

	void set_lightmap_size_hint(const Size2i &p_size);
	Size2i get_lightmap_size_hint() const;
	void clear_cache() const;

	Mesh(){}

};
}

#endif