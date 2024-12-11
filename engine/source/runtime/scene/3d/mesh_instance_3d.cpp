#include "mesh_instance_3d.h"
using namespace lain;
void lain::MeshInstance3D::set_mesh(const Ref<Mesh>& p_mesh) {
  mesh = p_mesh;
  if(mesh.is_valid()) {
    set_base(mesh->GetRID());
    _mesh_changed(); 
  } else {
    set_base(RID());
  }
  notify_property_list_changed();
}

Ref<Mesh> lain::MeshInstance3D::get_mesh() const {
  return mesh;
}
AABB MeshInstance3D::get_aabb() const {
	if (!mesh.is_null()) {
		return mesh->get_aabb();
	}

	return AABB();
}
void MeshInstance3D::_mesh_changed() {
	ERR_FAIL_COND(mesh.is_null());
	surface_override_materials.resize(mesh->get_surface_count());

	uint32_t initialize_bs_from = blend_shape_tracks.size();
	blend_shape_tracks.resize(mesh->get_blend_shape_count());

	for (uint32_t i = 0; i < blend_shape_tracks.size(); i++) {
		blend_shape_properties["blend_shapes/" + String(mesh->get_blend_shape_name(i))] = i;
		if (i < initialize_bs_from) {
			set_blend_shape_value(i, blend_shape_tracks[i]);
		} else {
			set_blend_shape_value(i, 0);
		}
	}

	int surface_count = mesh->get_surface_count();
	for (int surface_index = 0; surface_index < surface_count; ++surface_index) {
		if (surface_override_materials[surface_index].is_valid()) {
			RS::get_singleton()->instance_set_surface_override_material(get_instance(), surface_index, surface_override_materials[surface_index]->GetRID());
		}
	}

	update_gizmos();
}

float MeshInstance3D::get_blend_shape_value(int p_blend_shape) const {
	ERR_FAIL_COND_V(mesh.is_null(), 0.0);
	ERR_FAIL_INDEX_V(p_blend_shape, (int)blend_shape_tracks.size(), 0);
	return blend_shape_tracks[p_blend_shape];
}
void MeshInstance3D::set_blend_shape_value(int p_blend_shape, float p_value) {
	ERR_FAIL_COND(mesh.is_null());
	ERR_FAIL_INDEX(p_blend_shape, (int)blend_shape_tracks.size());
	blend_shape_tracks[p_blend_shape] = p_value;
	RS::get_singleton()->instance_set_blend_shape_weight(get_instance(), p_blend_shape, p_value);
}

void MeshInstance3D::_bind_methods(){
  ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &MeshInstance3D::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &MeshInstance3D::get_mesh);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
	
}

MeshInstance3D::MeshInstance3D() {
}

MeshInstance3D::~MeshInstance3D() {
}
