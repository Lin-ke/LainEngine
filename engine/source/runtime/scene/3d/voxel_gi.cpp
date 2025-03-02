#include "voxel_gi.h"
#include "voxelizer.h"
using namespace lain;

VoxelGI::BakeBeginFunc VoxelGI::bake_begin_function = nullptr;
VoxelGI::BakeStepFunc VoxelGI::bake_step_function = nullptr;
VoxelGI::BakeEndFunc VoxelGI::bake_end_function = nullptr;

VoxelGI::VoxelGI() {
	voxel_gi = RS::get_singleton()->voxel_gi_create();
	set_disable_scale(true);
}


VoxelGI::~VoxelGI() {
	RS::get_singleton()->free(voxel_gi);
}

void lain::VoxelGI::bake(GObject* p_from_node, bool p_create_visual_debug) {
  static const int subdiv_value[SUBDIV_MAX] = { 6, 7, 8, 9 };

	p_from_node = p_from_node ? p_from_node : get_parent();
	ERR_FAIL_NULL(p_from_node);

	float exposure_normalization = _get_camera_exposure_normalization();

	Voxelizer baker;

	baker.begin_bake(subdiv_value[subdiv], AABB(-size / 2, size), exposure_normalization);

	List<PlotMesh> mesh_list;

	_find_meshes(p_from_node, mesh_list);
}

void VoxelGI::set_probe_data(const Ref<VoxelGIData> &p_data) {
	if (p_data.is_valid()) {
		RS::get_singleton()->instance_set_base(get_instance(), p_data->GetRID());
		RS::get_singleton()->voxel_gi_set_baked_exposure_normalization(p_data->GetRID(), _get_camera_exposure_normalization());
	} else {
		RS::get_singleton()->instance_set_base(get_instance(), RID());
	}

	probe_data = p_data;
}


float VoxelGI::_get_camera_exposure_normalization() {
	float exposure_normalization = 1.0;
	// if (camera_attributes.is_valid()) {
	// 	exposure_normalization = camera_attributes->get_exposure_multiplier();
	// 	if (GLOBAL_GET("rendering/lights_and_shadows/use_physical_light_units")) {
	// 		exposure_normalization = camera_attributes->calculate_exposure_normalization();
	// 	}
	// }
	return exposure_normalization;
}


Ref<VoxelGIData> VoxelGI::get_probe_data() const {
	return probe_data;
}

AABB VoxelGI::get_aabb() const {
  return AABB(-size / 2, size);
}

void VoxelGIData::_set_data(const Dictionary &p_data) {
	ERR_FAIL_COND(!p_data.has("bounds"));
	ERR_FAIL_COND(!p_data.has("octree_size"));
	ERR_FAIL_COND(!p_data.has("octree_cells"));
	ERR_FAIL_COND(!p_data.has("octree_data"));
	ERR_FAIL_COND(!p_data.has("octree_df") && !p_data.has("octree_df_png"));
	ERR_FAIL_COND(!p_data.has("level_counts"));
	ERR_FAIL_COND(!p_data.has("to_cell_xform"));

	AABB bounds_new = p_data["bounds"];
	Vector3 octree_size_new = p_data["octree_size"];
	Vector<uint8_t> octree_cells = p_data["octree_cells"];
	Vector<uint8_t> octree_data = p_data["octree_data"];

	Vector<uint8_t> octree_df;
	if (p_data.has("octree_df")) {
		octree_df = p_data["octree_df"];
	} else if (p_data.has("octree_df_png")) {
		Vector<uint8_t> octree_df_png = p_data["octree_df_png"];
		Ref<Image> img;
		img.instantiate();
		Error err = img->load_png_from_buffer(octree_df_png);
		ERR_FAIL_COND(err != OK);
		ERR_FAIL_COND(img->get_format() != Image::FORMAT_L8);
		octree_df = img->get_data();
	}
	Vector<int> octree_levels = p_data["level_counts"];
	Transform3D to_cell_xform_new = p_data["to_cell_xform"];

	allocate(to_cell_xform_new, bounds_new, octree_size_new, octree_cells, octree_data, octree_df, octree_levels);
}

Dictionary VoxelGIData::_get_data() const {
	Dictionary d;
	d["bounds"] = get_bounds();
	Vector3i otsize = get_octree_size();
	d["octree_size"] = Vector3(otsize);
	d["octree_cells"] = get_octree_cells();
	d["octree_data"] = get_data_cells();
	if (otsize != Vector3i()) {
		Ref<Image> img = Image::create_from_data(otsize.x * otsize.y, otsize.z, false, Image::FORMAT_L8, get_distance_field());
		Vector<uint8_t> df_png = img->save_png_to_buffer();
		ERR_FAIL_COND_V(df_png.is_empty(), Dictionary());
		d["octree_df_png"] = df_png;
	} else {
		d["octree_df"] = Vector<uint8_t>();
	}

	d["level_counts"] = get_level_counts();
	d["to_cell_xform"] = get_to_cell_xform();
	return d;
}

void VoxelGIData::allocate(const Transform3D &p_to_cell_xform, const AABB &p_aabb, const Vector3 &p_octree_size, const Vector<uint8_t> &p_octree_cells, const Vector<uint8_t> &p_data_cells, const Vector<uint8_t> &p_distance_field, const Vector<int> &p_level_counts) {
	RS::get_singleton()->voxel_gi_allocate_data(probe, p_to_cell_xform, p_aabb, p_octree_size, p_octree_cells, p_data_cells, p_distance_field, p_level_counts);
	bounds = p_aabb;
	to_cell_xform = p_to_cell_xform;
	octree_size = p_octree_size;
}

AABB VoxelGIData::get_bounds() const {
	return bounds;
}

Vector3 VoxelGIData::get_octree_size() const {
	return octree_size;
}

Vector<uint8_t> VoxelGIData::get_octree_cells() const {
	return RS::get_singleton()->voxel_gi_get_octree_cells(probe);
}

Vector<uint8_t> VoxelGIData::get_data_cells() const {
	return RS::get_singleton()->voxel_gi_get_data_cells(probe);
}

Vector<uint8_t> VoxelGIData::get_distance_field() const {
	return RS::get_singleton()->voxel_gi_get_distance_field(probe);
}

Vector<int> VoxelGIData::get_level_counts() const {
	return RS::get_singleton()->voxel_gi_get_level_counts(probe);
}

Transform3D VoxelGIData::get_to_cell_xform() const {
	return to_cell_xform;
}

void VoxelGIData::set_dynamic_range(float p_range) {
	RS::get_singleton()->voxel_gi_set_dynamic_range(probe, p_range);
	dynamic_range = p_range;
}

float VoxelGIData::get_dynamic_range() const {
	return dynamic_range;
}

void VoxelGIData::set_propagation(float p_propagation) {
	RS::get_singleton()->voxel_gi_set_propagation(probe, p_propagation);
	propagation = p_propagation;
}

float VoxelGIData::get_propagation() const {
	return propagation;
}

void VoxelGIData::set_energy(float p_energy) {
	RS::get_singleton()->voxel_gi_set_energy(probe, p_energy);
	energy = p_energy;
}

float VoxelGIData::get_energy() const {
	return energy;
}

void VoxelGIData::set_bias(float p_bias) {
	RS::get_singleton()->voxel_gi_set_bias(probe, p_bias);
	bias = p_bias;
}

float VoxelGIData::get_bias() const {
	return bias;
}

void VoxelGIData::set_normal_bias(float p_normal_bias) {
	RS::get_singleton()->voxel_gi_set_normal_bias(probe, p_normal_bias);
	normal_bias = p_normal_bias;
}

float VoxelGIData::get_normal_bias() const {
	return normal_bias;
}

void VoxelGIData::set_interior(bool p_enable) {
	RS::get_singleton()->voxel_gi_set_interior(probe, p_enable);
	interior = p_enable;
}

bool VoxelGIData::is_interior() const {
	return interior;
}

void VoxelGIData::set_use_two_bounces(bool p_enable) {
	RS::get_singleton()->voxel_gi_set_use_two_bounces(probe, p_enable);
	use_two_bounces = p_enable;
}

bool VoxelGIData::is_using_two_bounces() const {
	return use_two_bounces;
}

RID VoxelGIData::GetRID() const {
	return probe;
}

void VoxelGIData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("allocate", "to_cell_xform", "aabb", "octree_size", "octree_cells", "data_cells", "distance_field", "level_counts"), &VoxelGIData::allocate);

	ClassDB::bind_method(D_METHOD("get_bounds"), &VoxelGIData::get_bounds);
	ClassDB::bind_method(D_METHOD("get_octree_size"), &VoxelGIData::get_octree_size);
	ClassDB::bind_method(D_METHOD("get_to_cell_xform"), &VoxelGIData::get_to_cell_xform);
	ClassDB::bind_method(D_METHOD("get_octree_cells"), &VoxelGIData::get_octree_cells);
	ClassDB::bind_method(D_METHOD("get_data_cells"), &VoxelGIData::get_data_cells);
	ClassDB::bind_method(D_METHOD("get_level_counts"), &VoxelGIData::get_level_counts);

	ClassDB::bind_method(D_METHOD("set_dynamic_range", "dynamic_range"), &VoxelGIData::set_dynamic_range);
	ClassDB::bind_method(D_METHOD("get_dynamic_range"), &VoxelGIData::get_dynamic_range);

	ClassDB::bind_method(D_METHOD("set_energy", "energy"), &VoxelGIData::set_energy);
	ClassDB::bind_method(D_METHOD("get_energy"), &VoxelGIData::get_energy);

	ClassDB::bind_method(D_METHOD("set_bias", "bias"), &VoxelGIData::set_bias);
	ClassDB::bind_method(D_METHOD("get_bias"), &VoxelGIData::get_bias);

	ClassDB::bind_method(D_METHOD("set_normal_bias", "bias"), &VoxelGIData::set_normal_bias);
	ClassDB::bind_method(D_METHOD("get_normal_bias"), &VoxelGIData::get_normal_bias);

	ClassDB::bind_method(D_METHOD("set_propagation", "propagation"), &VoxelGIData::set_propagation);
	ClassDB::bind_method(D_METHOD("get_propagation"), &VoxelGIData::get_propagation);

	ClassDB::bind_method(D_METHOD("set_interior", "interior"), &VoxelGIData::set_interior);
	ClassDB::bind_method(D_METHOD("is_interior"), &VoxelGIData::is_interior);

	ClassDB::bind_method(D_METHOD("set_use_two_bounces", "enable"), &VoxelGIData::set_use_two_bounces);
	ClassDB::bind_method(D_METHOD("is_using_two_bounces"), &VoxelGIData::is_using_two_bounces);

	ClassDB::bind_method(D_METHOD("_set_data", "data"), &VoxelGIData::_set_data);
	ClassDB::bind_method(D_METHOD("_get_data"), &VoxelGIData::_get_data);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "_data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL), "_set_data", "_get_data");

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "dynamic_range", PROPERTY_HINT_RANGE, "1,8,0.01"), "set_dynamic_range", "get_dynamic_range");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "energy", PROPERTY_HINT_RANGE, "0,64,0.01"), "set_energy", "get_energy");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bias", PROPERTY_HINT_RANGE, "0,8,0.01"), "set_bias", "get_bias");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "normal_bias", PROPERTY_HINT_RANGE, "0,8,0.01"), "set_normal_bias", "get_normal_bias");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "propagation", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_propagation", "get_propagation");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_two_bounces"), "set_use_two_bounces", "is_using_two_bounces");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "interior"), "set_interior", "is_interior");
}

VoxelGIData::VoxelGIData() {
	probe = RS::get_singleton()->voxel_gi_create();
}

VoxelGIData::~VoxelGIData() {
	RS::get_singleton()->free(probe);
}
