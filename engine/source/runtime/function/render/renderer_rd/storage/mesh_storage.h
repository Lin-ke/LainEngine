#ifndef MESH_STORAGE_IMPL_H
#define MESH_STORAGE_IMPL_H
#include "function/render/rendering_system/mesh_storage_api.h"
namespace lain::RendererRD {
class MeshStorage : public RendererMeshStorage {
  static MeshStorage* p_singleton;
	RID default_rd_storage_buffer;
  struct MeshInstance;
	// Mesh 由 surface 组成
	// surface 包含 各种数据， Material ，以及LOD (index buffer)
	// 持有instance的引用
  struct Mesh {
    struct Surface {
			RS::PrimitiveType primitive = RS::PRIMITIVE_POINTS;
			uint64_t format = 0;

			RID vertex_buffer;
			RID attribute_buffer;
			RID skin_buffer;
			uint32_t vertex_count = 0;
			uint32_t vertex_buffer_size = 0;
			uint32_t skin_buffer_size = 0;

			// A different pipeline needs to be allocated
			// depending on the inputs available in the
			// material.
			// There are never that many geometry/material
			// combinations, so a simple array is the most
			// cache-efficient structure.

			struct Version {
				uint64_t input_mask = 0;
				uint32_t current_buffer = 0;
				uint32_t previous_buffer = 0;
				bool input_motion_vectors = false;
				RD::VertexFormatID vertex_format = 0;
				RID vertex_array;
			};

			SpinLock version_lock; //needed to access versions
			Version *versions = nullptr; //allocated on demand
			uint32_t version_count = 0;

			RID index_buffer;
			RID index_array;
			uint32_t index_count = 0;

			struct LOD {
				float edge_length = 0.0;
				uint32_t index_count = 0;
				RID index_buffer; 
				RID index_array;
			};

			LOD *lods = nullptr;
			uint32_t lod_count = 0;

			AABB aabb;

			Vector<AABB> bone_aabbs;

			// Transform used in runtime bone AABBs compute.
			// As bone AABBs are saved in Mesh space, but bones animation is in Skeleton space.
			Transform3D mesh_to_skeleton_xform;

			Vector4 uv_scale;

			RID blend_shape_buffer;

			RID material;

			uint32_t render_index = 0;
			uint64_t render_pass = 0;

			uint32_t multimesh_render_index = 0;
			uint64_t multimesh_render_pass = 0;

			uint32_t particles_render_index = 0;
			uint64_t particles_render_pass = 0;

			RID uniform_set;
		};
    uint32_t blend_shape_count = 0;
		RS::BlendShapeMode blend_shape_mode = RS::BLEND_SHAPE_MODE_NORMALIZED;

		Surface **surfaces = nullptr;
		uint32_t surface_count = 0;

		bool has_bone_weights = false;

		AABB aabb;
		AABB custom_aabb;
		uint64_t skeleton_aabb_version = 0;
		RID skeleton_aabb_rid;

		Vector<RID> material_cache;

		List<MeshInstance *> instances;

		RID shadow_mesh;
		HashSet<Mesh *> shadow_owners; // 如果该纹理是shadow，则被哪些mesh拥有

		String path;

		Dependency dependency;
  
  };
  mutable RID_Owner<Mesh, true> mesh_owner;

/* Mesh Instance API */
	// Instance 具有skeleton
	// 具有此Mesh
	struct MeshInstance {
		Mesh *mesh = nullptr; // 这里竟然用指针而非RID()
		RID skeleton;
		struct Surface {
			RID vertex_buffer[2]; // swap buffer
			RID uniform_set[2];
			uint32_t current_buffer = 0;  // 0 or 1
			uint32_t previous_buffer = 0;
			uint64_t last_change = 0; // 上次变化时间，判断能否使用velocity

			Mesh::Surface::Version *versions = nullptr; //allocated on demand
			uint32_t version_count = 0;
		};
		LocalVector<Surface> surfaces;
		LocalVector<float> blend_weights;

		RID blend_weights_buffer; // buffer create 后记录 blend weights

		List<MeshInstance *>::Element *I = nullptr; //used to erase itself
		uint64_t skeleton_version = 0;
		bool dirty = false;
		bool weights_dirty = false;
		SelfList<MeshInstance> weight_update_list;
		SelfList<MeshInstance> array_update_list;
		Transform2D canvas_item_transform_2d;
		MeshInstance() :
				weight_update_list(this), array_update_list(this) {}
	};
	mutable RID_Owner<MeshInstance> mesh_instance_owner;
	// dirty list
	SelfList<MeshInstance>::List dirty_mesh_instance_weights;
	SelfList<MeshInstance>::List dirty_mesh_instance_arrays;


	struct SkeletonShader {
		struct PushConstant {
			uint32_t has_normal;
			uint32_t has_tangent;
			uint32_t has_skeleton;
			uint32_t has_blend_shape;

			uint32_t vertex_count;
			uint32_t vertex_stride;
			uint32_t skin_stride;
			uint32_t skin_weight_offset;

			uint32_t blend_shape_count;
			uint32_t normalized_blend_shapes;
			uint32_t normal_tangent_stride;
			uint32_t pad1;
			float skeleton_transform_x[2];
			float skeleton_transform_y[2];

			float skeleton_transform_offset[2];
			float inverse_transform_x[2];

			float inverse_transform_y[2];
			float inverse_transform_offset[2];
		};

		enum {
			UNIFORM_SET_INSTANCE = 0,
			UNIFORM_SET_SURFACE = 1,
			UNIFORM_SET_SKELETON = 2,
		};
		enum {
			SHADER_MODE_2D,
			SHADER_MODE_3D,
			SHADER_MODE_MAX
		};

		// SkeletonShaderRD shader;
		RID version;
		RID version_shader[SHADER_MODE_MAX]; // 根据不同的类型判断使用哪个
		RID pipeline[SHADER_MODE_MAX];

		RID default_skeleton_uniform_set;
	} skeleton_shader;
	struct Skeleton {
		bool use_2d = false;
		int size = 0;
		Vector<float> data;
		RID buffer;

		bool dirty = false;
		Skeleton *dirty_list = nullptr;
		Transform2D base_transform_2d;

		RID uniform_set_3d;
		RID uniform_set_mi;

		uint64_t version = 1;

		Dependency dependency;
	};

	mutable RID_Owner<Skeleton, true> skeleton_owner;


 public:
  MeshStorage() { p_singleton = this; }
  ~MeshStorage() {}
  static MeshStorage* get_singleton() { return p_singleton; };
	bool owns_mesh(RID p_rid) { return mesh_owner.owns(p_rid); };
	bool owns_multimesh(RID p_rid) { return false; };
  bool free(RID p_rid);
  virtual RID mesh_allocate() override;
  virtual void mesh_initialize(RID p_rid) override;
  virtual void mesh_free(RID p_rid) override;
	virtual void mesh_set_blend_shape_count(RID p_mesh, int p_blend_shape_count) override;
  virtual void mesh_add_surface(RID p_mesh, const RS::SurfaceData &p_surface) override;
  virtual int mesh_get_blend_shape_count(RID p_mesh) const override;
  virtual void mesh_set_blend_shape_mode(RID p_mesh, RS::BlendShapeMode p_mode) override;
  virtual RS::BlendShapeMode mesh_get_blend_shape_mode(RID p_mesh) const override;
  // update vertex buffer in mesh surface
	virtual void mesh_surface_update_vertex_region(RID p_mesh, int p_surface, int p_offset, const Vector<uint8_t> &p_data) override;
  // update attribute buffer in mesh surface
	virtual void mesh_surface_update_attribute_region(RID p_mesh, int p_surface, int p_offset, const Vector<uint8_t> &p_data) override;
  virtual void mesh_surface_update_skin_region(RID p_mesh, int p_surface, int p_offset, const Vector<uint8_t> &p_data) override;
  virtual void mesh_surface_set_material(RID p_mesh, int p_surface, RID p_material) override;
  virtual RID mesh_surface_get_material(RID p_mesh, int p_surface) const override;
  virtual RS::SurfaceData mesh_get_surface(RID p_mesh, int p_surface) const override;
  virtual int mesh_get_surface_count(RID p_mesh) const override;
  virtual void mesh_set_custom_aabb(RID p_mesh, const AABB &p_aabb) override;
  virtual AABB mesh_get_custom_aabb(RID p_mesh) const override;

  virtual AABB mesh_get_aabb(RID p_mesh, RID p_skeleton = RID()) override;

  virtual void mesh_set_path(RID p_mesh, const String &p_path) override;
  virtual String mesh_get_path(RID p_mesh) const override;
  virtual void mesh_set_shadow_mesh(RID p_mesh, RID p_shadow_mesh) override;

  virtual void mesh_clear(RID p_mesh) override;
  virtual bool mesh_needs_instance(RID p_mesh, bool p_has_skeleton) override;

  /* Mesh Instance */
  virtual RID mesh_instance_create(RID p_base) override;
  virtual void mesh_instance_free(RID p_rid) override;
  virtual void mesh_instance_set_skeleton(RID p_mesh_instance, RID p_skeleton) override;
  virtual void mesh_instance_set_blend_shape_weight(RID p_mesh_instance, int p_shape, float p_weight) override;
  // 会把需要更新的加入脏列表
	virtual void mesh_instance_check_for_update(RID p_mesh_instance) override;
  virtual void mesh_instance_set_canvas_item_transform(RID p_mesh_instance, const Transform2D &p_transform) override;
  virtual void update_mesh_instances() override;
  void _mesh_instance_clear(MeshInstance *p_mi);
  /* MultiMesh */
  /* MULTIMESH API */

	struct MultiMesh {
		RID mesh;
		int instances = 0;
		RS::MultimeshTransformFormat xform_format = RS::MULTIMESH_TRANSFORM_3D;
		bool uses_colors = false;
		bool uses_custom_data = false;
		int visible_instances = -1;
		AABB aabb;
		AABB custom_aabb;
		bool aabb_dirty = false; // if true, aabb needs to be recalculated
		bool buffer_set = false;
		bool motion_vectors_enabled = false;
		uint32_t motion_vectors_current_offset = 0;
		uint32_t motion_vectors_previous_offset = 0;
		uint64_t motion_vectors_last_change = -1;
		uint32_t stride_cache = 0;
		uint32_t color_offset_cache = 0;
		uint32_t custom_data_offset_cache = 0;

		Vector<float> data_cache; //used if individual setting is used // 	used when local 
		bool *data_cache_dirty_regions = nullptr; // is dirty (bool[])
		uint32_t data_cache_dirty_region_count = 0; // 需要处理几个
		bool *previous_data_cache_dirty_regions = nullptr;
		uint32_t previous_data_cache_dirty_region_count = 0;

		RID buffer; //storage buffer
		RID uniform_set_3d;
		RID uniform_set_2d;

		bool dirty = false; // has dirty flag
		MultiMesh *dirty_list = nullptr;

		Dependency dependency;
	};
	mutable RID_Owner<MultiMesh, true> multimesh_owner;


	// multi mesh 您希望在场景中创建同一网格的多个实例。您可以多次复制同一节点并手动调整变换。这可能是一个乏味的过程，而且结果可能看起来很机械。而且这种方法也不利于快速迭代。 
	// MultiMeshInstance3D 是该问题的可能解决方案之一。
	// 例如在土地上有很多树
	virtual RID multimesh_allocate() override;
	virtual void multimesh_initialize(RID p_rid) override;
	virtual void multimesh_free(RID p_rid) override;

	virtual void multimesh_allocate_data(RID p_multimesh, int p_instances, RS::MultimeshTransformFormat p_transform_format, bool p_use_colors = false, bool p_use_custom_data = false) override;

	virtual int multimesh_get_instance_count(RID p_multimesh) const override;

	virtual void multimesh_set_mesh(RID p_multimesh, RID p_mesh) override;
	// virtual void multimesh_instance_set_transform(RID p_multimesh, int p_index, const Transform3D &p_transform) override;
	// virtual void multimesh_instance_set_transform_2d(RID p_multimesh, int p_index, const Transform2D &p_transform) override;
	// virtual void multimesh_instance_set_color(RID p_multimesh, int p_index, const Color &p_color) override;
	// virtual void multimesh_instance_set_custom_data(RID p_multimesh, int p_index, const Color &p_color) override;

	virtual void multimesh_set_custom_aabb(RID p_multimesh, const AABB &p_aabb) override;
	virtual AABB multimesh_get_custom_aabb(RID p_multimesh) const override;

	virtual RID multimesh_get_mesh(RID p_multimesh) const override;

	virtual Transform3D multimesh_instance_get_transform(RID p_multimesh, int p_index) const override;
	virtual Transform2D multimesh_instance_get_transform_2d(RID p_multimesh, int p_index) const override;
	virtual Color multimesh_instance_get_color(RID p_multimesh, int p_index) const override;
	virtual Color multimesh_instance_get_custom_data(RID p_multimesh, int p_index) const override;

	virtual void multimesh_set_buffer(RID p_multimesh, const Vector<float> &p_buffer) override;
	virtual Vector<float> multimesh_get_buffer(RID p_multimesh) const override;

	virtual void multimesh_set_visible_instances(RID p_multimesh, int p_visible) override;
	virtual int multimesh_get_visible_instances(RID p_multimesh) const override;

	virtual AABB multimesh_get_aabb(RID p_multimesh) const override;
	/* SKELETON API */

	// virtual RID skeleton_allocate() override;
	// virtual void skeleton_initialize(RID p_rid) override;
	// virtual void skeleton_free(RID p_rid) override;

	// virtual void skeleton_allocate_data(RID p_skeleton, int p_bones, bool p_2d_skeleton = false) override;
	// virtual int skeleton_get_bone_count(RID p_skeleton) const override;
	// virtual void skeleton_bone_set_transform(RID p_skeleton, int p_bone, const Transform3D &p_transform) override;
	// virtual Transform3D skeleton_bone_get_transform(RID p_skeleton, int p_bone) const override;
	// virtual void skeleton_bone_set_transform_2d(RID p_skeleton, int p_bone, const Transform2D &p_transform) override;
	// virtual Transform2D skeleton_bone_get_transform_2d(RID p_skeleton, int p_bone) const override;
	// virtual void skeleton_set_base_transform_2d(RID p_skeleton, const Transform2D &p_base_transform) override;

	// virtual void skeleton_update_dependency(RID p_base, DependencyTracker *p_instance) override;
	private:
	void _mesh_instance_add_surface(MeshInstance *mi, Mesh *mesh, uint32_t p_surface);
	void _mesh_instance_add_surface_buffer(MeshInstance *mi, Mesh *mesh, MeshInstance::Surface *s, uint32_t p_surface, uint32_t p_buffer_index);
	void _update_dirty_multimeshes();
	_FORCE_INLINE_ void _multimesh_re_create_aabb(MultiMesh *multimesh, const float *p_data, int p_instances);
	_FORCE_INLINE_ void _multimesh_make_local(MultiMesh *multimesh) const;
	_FORCE_INLINE_ void _multimesh_mark_dirty(MultiMesh *multimesh, int p_index, bool p_aabb);
	_FORCE_INLINE_ void _multimesh_mark_all_dirty(MultiMesh *multimesh, bool p_data, bool p_aabb);
	_FORCE_INLINE_ void _multimesh_enable_motion_vectors(MultiMesh *multimesh);

	MultiMesh *multimesh_dirty_list = nullptr;

  
};
}  // namespace lain::RendererRD
#endif