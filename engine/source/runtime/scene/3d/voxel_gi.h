#ifndef __VOXEL_GI_H__
#define __VOXEL_GI_H__

#include "scene/3d/visual_instance_3d.h"
#include "mesh_instance_3d.h"

namespace lain {

class VoxelGIData : public Resource {
  LCLASS(VoxelGIData, Resource);

  RID probe;
	void _set_data(const Dictionary &p_data);
	Dictionary _get_data() const;

	Transform3D to_cell_xform;
	AABB bounds;
	Vector3 octree_size;

	float dynamic_range = 2.0;
	float energy = 1.0;
	float bias = 1.5;
	float normal_bias = 0.0;
	float propagation = 0.5;
	bool interior = false;
	bool use_two_bounces = true;
 protected:
  static void _bind_methods();

 public:
  void allocate(const Transform3D& p_to_cell_xform, const AABB& p_aabb, const Vector3& p_octree_size, const Vector<uint8_t>& p_octree_cells,
                const Vector<uint8_t>& p_data_cells, const Vector<uint8_t>& p_distance_field, const Vector<int>& p_level_counts);
  AABB get_bounds() const;
  Vector3 get_octree_size() const;
  Vector<uint8_t> get_octree_cells() const;
  Vector<uint8_t> get_data_cells() const;
  Vector<uint8_t> get_distance_field() const;
  Vector<int> get_level_counts() const;
  Transform3D get_to_cell_xform() const;

  void set_dynamic_range(float p_range);
  float get_dynamic_range() const;

  void set_propagation(float p_propagation);
  float get_propagation() const;

  void set_energy(float p_energy);
  float get_energy() const;

  void set_bias(float p_bias);
  float get_bias() const;

  void set_normal_bias(float p_normal_bias);
  float get_normal_bias() const;

  void set_interior(bool p_enable);
  bool is_interior() const;

  void set_use_two_bounces(bool p_enable);
  bool is_using_two_bounces() const;

  virtual RID GetRID() const override;

  VoxelGIData();
  ~VoxelGIData();
};
class VoxelGI : public VisualInstance3D {
  LCLASS(VoxelGI, VisualInstance3D);

 public:
  enum Subdiv {
    SUBDIV_64,
    SUBDIV_128,
    SUBDIV_256,
    SUBDIV_512,
    SUBDIV_MAX

  };
  
	struct PlotMesh {
		Ref<Material> override_material;
		Vector<Ref<Material>> instance_materials;
		Ref<Mesh> mesh;
		Transform3D local_xform;
	};


  typedef void (*BakeBeginFunc)(int);
  typedef void (*BakeStepFunc)(int, const String&);
  typedef void (*BakeEndFunc)();
  static BakeBeginFunc bake_begin_function;
  static BakeStepFunc bake_step_function;
  static BakeEndFunc bake_end_function;

 private:
  Subdiv subdiv = SUBDIV_128;
  Ref<VoxelGIData> probe_data;
  RID voxel_gi;
  Vector3 size = Vector3(20, 20, 20);
	Ref<CameraAttributes> camera_attributes;
	float _get_camera_exposure_normalization();
  void _find_meshes(GObject *p_at_node, List<PlotMesh> &plot_meshes);
  
 public:
  void set_probe_data(const Ref<VoxelGIData>& p_data);
  Ref<VoxelGIData> get_probe_data() const;

  virtual AABB get_aabb() const override;
  VoxelGI();
  ~VoxelGI();
	void bake(GObject *p_from_node = nullptr, bool p_create_visual_debug = false);

};
};  // namespace lain
#endif