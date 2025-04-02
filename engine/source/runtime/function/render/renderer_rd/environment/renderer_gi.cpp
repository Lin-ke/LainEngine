#include "renderer_gi.h"
#include "../storage/light_storage.h"
#include "../storage/material_storage.h"
#include "../renderer_scene_render_rd.h"
namespace lain::RendererRD {
GI* GI::p_singleton = nullptr;
void GI::init(SkyRD * p_sky) {
	RendererRD::TextureStorage *texture_storage = RendererRD::TextureStorage::get_singleton();
	RendererRD::MaterialStorage *material_storage = RendererRD::MaterialStorage::get_singleton();


	/* GI */	
	{
		//kinda complicated to compute the amount of slots, we try to use as many as we can

		voxel_gi_lights = memnew_arr(VoxelGILight, voxel_gi_max_lights);
		voxel_gi_lights_uniform = RD::get_singleton()->uniform_buffer_create(voxel_gi_max_lights * sizeof(VoxelGILight));
		voxel_gi_quality = RS::VoxelGIQuality(CLAMP(int(GLOBAL_GET("rendering/global_illumination/voxel_gi/quality")), 0, 1));

		String defines = "\n#define MAX_LIGHTS " + itos(voxel_gi_max_lights) + "\n";

		Vector<String> versions;
		versions.push_back("\n#define MODE_COMPUTE_LIGHT\n");
		versions.push_back("\n#define MODE_SECOND_BOUNCE\n");
		versions.push_back("\n#define MODE_UPDATE_MIPMAPS\n");
		versions.push_back("\n#define MODE_WRITE_TEXTURE\n");
		versions.push_back("\n#define MODE_DYNAMIC\n#define MODE_DYNAMIC_LIGHTING\n");
		versions.push_back("\n#define MODE_DYNAMIC\n#define MODE_DYNAMIC_SHRINK\n#define MODE_DYNAMIC_SHRINK_WRITE\n");
		versions.push_back("\n#define MODE_DYNAMIC\n#define MODE_DYNAMIC_SHRINK\n#define MODE_DYNAMIC_SHRINK_PLOT\n");
		versions.push_back("\n#define MODE_DYNAMIC\n#define MODE_DYNAMIC_SHRINK\n#define MODE_DYNAMIC_SHRINK_PLOT\n#define MODE_DYNAMIC_SHRINK_WRITE\n");

		voxel_gi_shader.initialize(versions, defines);
		voxel_gi_lighting_shader_version = voxel_gi_shader.version_create();
		for (int i = 0; i < VOXEL_GI_SHADER_VERSION_MAX; i++) {
			voxel_gi_lighting_shader_version_shaders[i] = voxel_gi_shader.version_get_shader(voxel_gi_lighting_shader_version, i);
			voxel_gi_lighting_shader_version_pipelines[i] = RD::get_singleton()->compute_pipeline_create(voxel_gi_lighting_shader_version_shaders[i]);
		}
	}

	// {
	// 	String defines;
	// 	Vector<String> versions;
	// 	versions.push_back("\n#define MODE_DEBUG_COLOR\n");
	// 	versions.push_back("\n#define MODE_DEBUG_LIGHT\n");
	// 	versions.push_back("\n#define MODE_DEBUG_EMISSION\n");
	// 	versions.push_back("\n#define MODE_DEBUG_LIGHT\n#define MODE_DEBUG_LIGHT_FULL\n");

	// 	voxel_gi_debug_shader.initialize(versions, defines);
	// 	voxel_gi_debug_shader_version = voxel_gi_debug_shader.version_create();
	// 	for (int i = 0; i < VOXEL_GI_DEBUG_MAX; i++) {
	// 		voxel_gi_debug_shader_version_shaders[i] = voxel_gi_debug_shader.version_get_shader(voxel_gi_debug_shader_version, i);

	// 		RD::PipelineRasterizationState rs;
	// 		rs.cull_mode = RD::POLYGON_CULL_FRONT;
	// 		RD::PipelineDepthStencilState ds;
	// 		ds.enable_depth_test = true;
	// 		ds.enable_depth_write = true;
	// 		ds.depth_compare_operator = RD::COMPARE_OP_GREATER_OR_EQUAL;

	// 		voxel_gi_debug_shader_version_pipelines[i].setup(voxel_gi_debug_shader_version_shaders[i], RD::RENDER_PRIMITIVE_TRIANGLES, rs, RD::PipelineMultisampleState(), ds, RD::PipelineColorBlendState::create_disabled(), 0);
	// 	}
	// }

	/* SDGFI */

		// {
		// 	Vector<String> preprocess_modes;
		// 	preprocess_modes.push_back("\n#define MODE_SCROLL\n");
		// 	preprocess_modes.push_back("\n#define MODE_SCROLL_OCCLUSION\n");
		// 	preprocess_modes.push_back("\n#define MODE_INITIALIZE_JUMP_FLOOD\n");
		// 	preprocess_modes.push_back("\n#define MODE_INITIALIZE_JUMP_FLOOD_HALF\n");
		// 	preprocess_modes.push_back("\n#define MODE_JUMPFLOOD\n");
		// 	preprocess_modes.push_back("\n#define MODE_JUMPFLOOD_OPTIMIZED\n");
		// 	preprocess_modes.push_back("\n#define MODE_UPSCALE_JUMP_FLOOD\n");
		// 	preprocess_modes.push_back("\n#define MODE_OCCLUSION\n");
		// 	preprocess_modes.push_back("\n#define MODE_STORE\n");
		// 	String defines = "\n#define OCCLUSION_SIZE " + itos(SDFGI::CASCADE_SIZE / SDFGI::PROBE_DIVISOR) + "\n";
		// 	sdfgi_shader.preprocess.initialize(preprocess_modes, defines);
		// 	sdfgi_shader.preprocess_shader = sdfgi_shader.preprocess.version_create();
		// 	for (int i = 0; i < SDFGIShader::PRE_PROCESS_MAX; i++) {
		// 		sdfgi_shader.preprocess_pipeline[i] = RD::get_singleton()->compute_pipeline_create(sdfgi_shader.preprocess.version_get_shader(sdfgi_shader.preprocess_shader, i));
		// 	}
		// }



  default_voxel_gi_buffer = RD::get_singleton()->uniform_buffer_create(sizeof(VoxelGIData) * MAX_VOXEL_GI_INSTANCES);
  sdfgi_ubo = RD::get_singleton()->uniform_buffer_create(sizeof(SDFGIData));
}
RID GI::voxel_gi_allocate() {

  auto rid =  voxel_gi_owner.allocate_rid();
	bool ok = voxel_gi_owner.owns(rid);
	return rid;
}

void GI::voxel_gi_free(RID p_voxel_gi) {

  voxel_gi_allocate_data(p_voxel_gi, Transform3D(), AABB(), Vector3i(), Vector<uint8_t>(), Vector<uint8_t>(), Vector<uint8_t>(), Vector<int>());  //deallocate
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  voxel_gi->dependency.deleted_notify(p_voxel_gi);
  voxel_gi_owner.free(p_voxel_gi);
}
void GI::voxel_gi_allocate_data(RID p_voxel_gi, const Transform3D& p_to_cell_xform, const AABB& p_aabb, const Vector3i& p_octree_size, const Vector<uint8_t>& p_octree_cells,
                                const Vector<uint8_t>& p_data_cells, const Vector<uint8_t>& p_distance_field, const Vector<int>& p_level_counts) {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL(voxel_gi);

  if (voxel_gi->octree_buffer.is_valid()) {
    RD::get_singleton()->free(voxel_gi->octree_buffer);
    RD::get_singleton()->free(voxel_gi->data_buffer);
    if (voxel_gi->sdf_texture.is_valid()) {
      RD::get_singleton()->free(voxel_gi->sdf_texture);
    }

    voxel_gi->sdf_texture = RID();
    voxel_gi->octree_buffer = RID();
    voxel_gi->data_buffer = RID();
    voxel_gi->octree_buffer_size = 0;
    voxel_gi->data_buffer_size = 0;
    voxel_gi->cell_count = 0;
  }

  voxel_gi->to_cell_xform = p_to_cell_xform;
  voxel_gi->bounds = p_aabb;
  voxel_gi->octree_size = p_octree_size;
  voxel_gi->level_counts = p_level_counts;

  if (p_octree_cells.size()) {
    ERR_FAIL_COND(p_octree_cells.size() % 32 != 0);  //cells size must be a multiple of 32

    uint32_t cell_count = p_octree_cells.size() / 32;

    ERR_FAIL_COND(p_data_cells.size() != (int)cell_count * 16);  //see that data size matches

    voxel_gi->cell_count = cell_count;
    voxel_gi->octree_buffer = RD::get_singleton()->storage_buffer_create(p_octree_cells.size(), p_octree_cells);
    voxel_gi->octree_buffer_size = p_octree_cells.size();
    voxel_gi->data_buffer = RD::get_singleton()->storage_buffer_create(p_data_cells.size(), p_data_cells);
    voxel_gi->data_buffer_size = p_data_cells.size();

    if (p_distance_field.size()) {
      RD::TextureFormat tf;
      tf.format = RD::DATA_FORMAT_R8_UNORM;
      tf.width = voxel_gi->octree_size.x;
      tf.height = voxel_gi->octree_size.y;
      tf.depth = voxel_gi->octree_size.z;
      tf.texture_type = RD::TEXTURE_TYPE_3D;
      tf.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
      Vector<Vector<uint8_t>> s;
      s.push_back(p_distance_field);
      voxel_gi->sdf_texture = RD::get_singleton()->texture_create(tf, RD::TextureView(), s);
      RD::get_singleton()->set_resource_name(voxel_gi->sdf_texture, "VoxelGI SDF Texture");
    }

    voxel_gi->version++;
    voxel_gi->data_version++;

    voxel_gi->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_AABB);
  }
}

AABB GI::voxel_gi_get_bounds(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, AABB());

  return voxel_gi->bounds;
}

Vector3i GI::voxel_gi_get_octree_size(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, Vector3i());
  return voxel_gi->octree_size;
}

Vector<uint8_t> GI::voxel_gi_get_octree_cells(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, Vector<uint8_t>());

  if (voxel_gi->octree_buffer.is_valid()) {
    return RD::get_singleton()->buffer_get_data(voxel_gi->octree_buffer);
  }
  return Vector<uint8_t>();
}

Vector<uint8_t> GI::voxel_gi_get_data_cells(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, Vector<uint8_t>());

  if (voxel_gi->data_buffer.is_valid()) {
    return RD::get_singleton()->buffer_get_data(voxel_gi->data_buffer);
  }
  return Vector<uint8_t>();
}

Vector<uint8_t> GI::voxel_gi_get_distance_field(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, Vector<uint8_t>());

  if (voxel_gi->data_buffer.is_valid()) {
    return RD::get_singleton()->texture_get_data(voxel_gi->sdf_texture, 0);
  }
  return Vector<uint8_t>();
}

Vector<int> GI::voxel_gi_get_level_counts(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, Vector<int>());

  return voxel_gi->level_counts;
}

Transform3D GI::voxel_gi_get_to_cell_xform(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, Transform3D());

  return voxel_gi->to_cell_xform;
}

void GI::voxel_gi_set_dynamic_range(RID p_voxel_gi, float p_range) {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL(voxel_gi);

  voxel_gi->dynamic_range = p_range;
  voxel_gi->version++;
}

float GI::voxel_gi_get_dynamic_range(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, 0);

  return voxel_gi->dynamic_range;
}

void GI::voxel_gi_set_propagation(RID p_voxel_gi, float p_range) {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL(voxel_gi);

  voxel_gi->propagation = p_range;
  voxel_gi->version++;
}

float GI::voxel_gi_get_propagation(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, 0);
  return voxel_gi->propagation;
}

void GI::voxel_gi_set_energy(RID p_voxel_gi, float p_energy) {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL(voxel_gi);

  voxel_gi->energy = p_energy;
}

float GI::voxel_gi_get_energy(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, 0);
  return voxel_gi->energy;
}

void GI::voxel_gi_set_baked_exposure_normalization(RID p_voxel_gi, float p_baked_exposure) {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL(voxel_gi);

  voxel_gi->baked_exposure = p_baked_exposure;
}

float GI::voxel_gi_get_baked_exposure_normalization(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, 0);
  return voxel_gi->baked_exposure;
}

void GI::voxel_gi_set_bias(RID p_voxel_gi, float p_bias) {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL(voxel_gi);

  voxel_gi->bias = p_bias;
}

float GI::voxel_gi_get_bias(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, 0);
  return voxel_gi->bias;
}

void GI::voxel_gi_set_normal_bias(RID p_voxel_gi, float p_normal_bias) {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL(voxel_gi);

  voxel_gi->normal_bias = p_normal_bias;
}

float GI::voxel_gi_get_normal_bias(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, 0);
  return voxel_gi->normal_bias;
}

void GI::voxel_gi_set_interior(RID p_voxel_gi, bool p_enable) {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL(voxel_gi);

  voxel_gi->interior = p_enable;
}

void GI::voxel_gi_set_use_two_bounces(RID p_voxel_gi, bool p_enable) {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL(voxel_gi);

  voxel_gi->use_two_bounces = p_enable;
  voxel_gi->version++;
}

bool GI::voxel_gi_is_using_two_bounces(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, false);
  return voxel_gi->use_two_bounces;
}

bool GI::voxel_gi_is_interior(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, 0);
  return voxel_gi->interior;
}

uint32_t GI::voxel_gi_get_version(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, 0);
  return voxel_gi->version;
}

uint32_t GI::voxel_gi_get_data_version(RID p_voxel_gi) {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, 0);
  return voxel_gi->data_version;
}

RID GI::voxel_gi_get_octree_buffer(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, RID());
  return voxel_gi->octree_buffer;
}

RID GI::voxel_gi_get_data_buffer(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, RID());
  return voxel_gi->data_buffer;
}

RID GI::voxel_gi_get_sdf_texture(RID p_voxel_gi) {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, RID());

  return voxel_gi->sdf_texture;
}

Dependency* GI::voxel_gi_get_dependency(RID p_voxel_gi) const {
  VoxelGI* voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
  ERR_FAIL_NULL_V(voxel_gi, nullptr);

  return &voxel_gi->dependency;
}


RID GI::voxel_gi_instance_create(RID p_base) {
	VoxelGIInstance voxel_gi;
	voxel_gi.gi = this;
	voxel_gi.probe = p_base;
	RID rid = voxel_gi_instance_owner.make_rid(voxel_gi);
	return rid;
}

void GI::voxel_gi_instance_free(RID p_rid) {
	GI::VoxelGIInstance *voxel_gi = voxel_gi_instance_owner.get_or_null(p_rid);
	voxel_gi->free_resources();
	voxel_gi_instance_owner.free(p_rid);
}

void GI::voxel_gi_instance_set_transform_to_data(RID p_probe, const Transform3D &p_xform) {
	VoxelGIInstance *voxel_gi = voxel_gi_instance_owner.get_or_null(p_probe);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->transform = p_xform;
}

bool GI::voxel_gi_needs_update(RID p_probe) const {
	VoxelGIInstance *voxel_gi = voxel_gi_instance_owner.get_or_null(p_probe);
	ERR_FAIL_NULL_V(voxel_gi, false);

	return voxel_gi->last_probe_version != voxel_gi_get_version(voxel_gi->probe);
}

void GI::voxel_gi_update(RID p_probe, bool p_update_light_instances, const Vector<RID> &p_light_instances, const PagedArray<RenderGeometryInstance *> &p_dynamic_objects) {
	VoxelGIInstance *voxel_gi = voxel_gi_instance_owner.get_or_null(p_probe);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->update(p_update_light_instances, p_light_instances, p_dynamic_objects);
}

void GI::debug_voxel_gi(RID p_voxel_gi, RD::DrawListID p_draw_list, RID p_framebuffer, const Projection &p_camera_with_transform, bool p_lighting, bool p_emission, float p_alpha) {
	VoxelGIInstance *voxel_gi = voxel_gi_instance_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->debug(p_draw_list, p_framebuffer, p_camera_with_transform, p_lighting, p_emission, p_alpha);
}

void GI::sdfgi_reset() {
  sdfgi_current_version++;
}

GI::GI() {
	p_singleton = this;

	// sdfgi_ray_count = RS::EnvironmentSDFGIRayCount(CLAMP(int32_t(GLOBAL_GET("rendering/global_illumination/sdfgi/probe_ray_count")), 0, int32_t(RS::ENV_SDFGI_RAY_COUNT_MAX - 1)));
	// sdfgi_frames_to_converge = RS::EnvironmentSDFGIFramesToConverge(CLAMP(int32_t(GLOBAL_GET("rendering/global_illumination/sdfgi/frames_to_converge")), 0, int32_t(RS::ENV_SDFGI_CONVERGE_MAX - 1)));
	// sdfgi_frames_to_update_light = RS::EnvironmentSDFGIFramesToUpdateLight(CLAMP(int32_t(GLOBAL_GET("rendering/global_illumination/sdfgi/frames_to_update_lights")), 0, int32_t(RS::ENV_SDFGI_UPDATE_LIGHT_MAX - 1)));
}
GI::~GI(){
	p_singleton = nullptr;
}

void GI::VoxelGIInstance::free_resources() {
	if (texture.is_valid()) {
		RD::get_singleton()->free(texture);
		RD::get_singleton()->free(write_buffer);

		texture = RID();
		write_buffer = RID();
		mipmaps.clear();
	}

	for (int i = 0; i < dynamic_maps.size(); i++) {
		RD::get_singleton()->free(dynamic_maps[i].texture);
		RD::get_singleton()->free(dynamic_maps[i].depth);

		// these only exist on the first level...
		if (dynamic_maps[i].fb_depth.is_valid()) {
			RD::get_singleton()->free(dynamic_maps[i].fb_depth);
		}
		if (dynamic_maps[i].albedo.is_valid()) {
			RD::get_singleton()->free(dynamic_maps[i].albedo);
		}
		if (dynamic_maps[i].normal.is_valid()) {
			RD::get_singleton()->free(dynamic_maps[i].normal);
		}
		if (dynamic_maps[i].orm.is_valid()) {
			RD::get_singleton()->free(dynamic_maps[i].orm);
		}
	}
	dynamic_maps.clear();
}

void GI::VoxelGIInstance::update(bool p_update_light_instances, const Vector<RID> &p_light_instances, const PagedArray<RenderGeometryInstance *> &p_dynamic_objects) {
	RendererRD::LightStorage *light_storage = RendererRD::LightStorage::get_singleton();
	RendererRD::MaterialStorage *material_storage = RendererRD::MaterialStorage::get_singleton();

	uint32_t data_version = gi->voxel_gi_get_data_version(probe);

	// (RE)CREATE IF NEEDED

	if (last_probe_data_version != data_version) {
		//need to re-create everything
		free_resources();

		Vector3i octree_size = gi->voxel_gi_get_octree_size(probe);

		if (octree_size != Vector3i()) {
			//can create a 3D texture
			Vector<int> levels = gi->voxel_gi_get_level_counts(probe);

			RD::TextureFormat tf;
			tf.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
			tf.width = octree_size.x;
			tf.height = octree_size.y;
			tf.depth = octree_size.z;
			tf.texture_type = RD::TEXTURE_TYPE_3D;
			tf.mipmaps = levels.size(); // mipmap 就是level

			tf.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT;

			texture = RD::get_singleton()->texture_create(tf, RD::TextureView());
			RD::get_singleton()->set_resource_name(texture, "VoxelGI Instance Texture");

			RD::get_singleton()->texture_clear(texture, Color(0, 0, 0, 0), 0, levels.size(), 0, 1);

			{
				int total_elements = 0;
				for (int i = 0; i < levels.size(); i++) {
					total_elements += levels[i];
				}

				write_buffer = RD::get_singleton()->storage_buffer_create(total_elements * 16);
			}

			for (int i = 0; i < levels.size(); i++) {
				VoxelGIInstance::Mipmap mipmap;
				mipmap.texture = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), texture, 0, i, 1, RD::TEXTURE_SLICE_3D);
				mipmap.level = levels.size() - i - 1;
				mipmap.cell_offset = 0;
				for (uint32_t j = 0; j < mipmap.level; j++) {
					mipmap.cell_offset += levels[j];
				}
				mipmap.cell_count = levels[mipmap.level];

				Vector<RD::Uniform> uniforms;
				{
					RD::Uniform u;
					u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
					u.binding = 1;
					u.append_id(gi->voxel_gi_get_octree_buffer(probe));
					uniforms.push_back(u);
				}
				{
					RD::Uniform u;
					u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
					u.binding = 2;
					u.append_id(gi->voxel_gi_get_data_buffer(probe));
					uniforms.push_back(u);
				}

				{
					RD::Uniform u;
					u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
					u.binding = 4;
					u.append_id(write_buffer);
					uniforms.push_back(u);
				}
				{
					RD::Uniform u;
					u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
					u.binding = 9;
					u.append_id(gi->voxel_gi_get_sdf_texture(probe));
					uniforms.push_back(u);
				}
				{
					RD::Uniform u;
					u.uniform_type = RD::UNIFORM_TYPE_SAMPLER;
					u.binding = 10;
					u.append_id(material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED));
					uniforms.push_back(u);
				}

				{
					Vector<RD::Uniform> copy_uniforms = uniforms;
					if (i == 0) {
						{
							RD::Uniform u;
							u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
							u.binding = 3;
							u.append_id(gi->voxel_gi_lights_uniform);
							copy_uniforms.push_back(u);
						}

						mipmap.uniform_set = RD::get_singleton()->uniform_set_create(copy_uniforms, gi->voxel_gi_lighting_shader_version_shaders[VOXEL_GI_SHADER_VERSION_COMPUTE_LIGHT], 0);

						copy_uniforms = uniforms; //restore

						{
							RD::Uniform u;
							u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
							u.binding = 5;
							u.append_id(texture);
							copy_uniforms.push_back(u);
						}
						mipmap.second_bounce_uniform_set = RD::get_singleton()->uniform_set_create(copy_uniforms, gi->voxel_gi_lighting_shader_version_shaders[VOXEL_GI_SHADER_VERSION_COMPUTE_SECOND_BOUNCE], 0);
					} else {
						mipmap.uniform_set = RD::get_singleton()->uniform_set_create(copy_uniforms, gi->voxel_gi_lighting_shader_version_shaders[VOXEL_GI_SHADER_VERSION_COMPUTE_MIPMAP], 0);
					}
				}

				{
					RD::Uniform u;
					u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
					u.binding = 5;
					u.append_id(mipmap.texture);
					uniforms.push_back(u);
				}

				mipmap.write_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->voxel_gi_lighting_shader_version_shaders[VOXEL_GI_SHADER_VERSION_WRITE_TEXTURE], 0);

				mipmaps.push_back(mipmap);
			}

			{
				uint32_t dynamic_map_size = MAX(MAX(octree_size.x, octree_size.y), octree_size.z);
				uint32_t oversample = nearest_power_of_2_templated(4);
				int mipmap_index = 0;

				while (mipmap_index < mipmaps.size()) {
					VoxelGIInstance::DynamicMap dmap;

					if (oversample > 0) {
						dmap.size = dynamic_map_size * (1 << oversample);
						dmap.mipmap = -1;
						oversample--;
					} else {
						dmap.size = dynamic_map_size >> mipmap_index;
						dmap.mipmap = mipmap_index;
						mipmap_index++;
					}

					RD::TextureFormat dtf;
					dtf.width = dmap.size;
					dtf.height = dmap.size;
					dtf.format = RD::DATA_FORMAT_R16G16B16A16_SFLOAT;
					dtf.usage_bits = RD::TEXTURE_USAGE_STORAGE_BIT;

					if (dynamic_maps.size() == 0) {
						dtf.usage_bits |= RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;
					}
					dmap.texture = RD::get_singleton()->texture_create(dtf, RD::TextureView());
					RD::get_singleton()->set_resource_name(dmap.texture, "VoxelGI Instance DMap Texture");

					if (dynamic_maps.size() == 0) {
						// Render depth for first one.
						// Use 16-bit depth when supported to improve performance.
						dtf.format = RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_D16_UNORM, RD::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) ? RD::DATA_FORMAT_D16_UNORM : RD::DATA_FORMAT_X8_D24_UNORM_PACK32;
						dtf.usage_bits = RD::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
						dmap.fb_depth = RD::get_singleton()->texture_create(dtf, RD::TextureView());
						RD::get_singleton()->set_resource_name(dmap.fb_depth, "VoxelGI Instance DMap FB Depth");
					}

					//just use depth as-is
					dtf.format = RD::DATA_FORMAT_R32_SFLOAT;
					dtf.usage_bits = RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;

					dmap.depth = RD::get_singleton()->texture_create(dtf, RD::TextureView());
					RD::get_singleton()->set_resource_name(dmap.depth, "VoxelGI Instance DMap Depth");

					if (dynamic_maps.size() == 0) {
						dtf.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
						dtf.usage_bits = RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;
						dmap.albedo = RD::get_singleton()->texture_create(dtf, RD::TextureView());
						RD::get_singleton()->set_resource_name(dmap.albedo, "VoxelGI Instance DMap Albedo");
						dmap.normal = RD::get_singleton()->texture_create(dtf, RD::TextureView());
						RD::get_singleton()->set_resource_name(dmap.normal, "VoxelGI Instance DMap Normal");
						dmap.orm = RD::get_singleton()->texture_create(dtf, RD::TextureView());
						RD::get_singleton()->set_resource_name(dmap.orm, "VoxelGI Instance DMap ORM");

						Vector<RID> fb;
						fb.push_back(dmap.albedo);
						fb.push_back(dmap.normal);
						fb.push_back(dmap.orm);
						fb.push_back(dmap.texture); //emission
						fb.push_back(dmap.depth);
						fb.push_back(dmap.fb_depth);

						dmap.fb = RD::get_singleton()->framebuffer_create(fb);

						{
							Vector<RD::Uniform> uniforms;
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
								u.binding = 3;
								u.append_id(gi->voxel_gi_lights_uniform);
								uniforms.push_back(u);
							}

							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 5;
								u.append_id(dmap.albedo);
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 6;
								u.append_id(dmap.normal);
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 7;
								u.append_id(dmap.orm);
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
								u.binding = 8;
								u.append_id(dmap.fb_depth);
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
								u.binding = 9;
								u.append_id(gi->voxel_gi_get_sdf_texture(probe));
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_SAMPLER;
								u.binding = 10;
								u.append_id(material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED));
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 11;
								u.append_id(dmap.texture);
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 12;
								u.append_id(dmap.depth);
								uniforms.push_back(u);
							}

							dmap.uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->voxel_gi_lighting_shader_version_shaders[VOXEL_GI_SHADER_VERSION_DYNAMIC_OBJECT_LIGHTING], 0);
						}
					} else {
						bool plot = dmap.mipmap >= 0;
						bool write = dmap.mipmap < (mipmaps.size() - 1);

						Vector<RD::Uniform> uniforms;

						{
							RD::Uniform u;
							u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
							u.binding = 5;
							u.append_id(dynamic_maps[dynamic_maps.size() - 1].texture);
							uniforms.push_back(u);
						}
						{
							RD::Uniform u;
							u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
							u.binding = 6;
							u.append_id(dynamic_maps[dynamic_maps.size() - 1].depth);
							uniforms.push_back(u);
						}

						if (write) {
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 7;
								u.append_id(dmap.texture);
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 8;
								u.append_id(dmap.depth);
								uniforms.push_back(u);
							}
						}

						{
							RD::Uniform u;
							u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
							u.binding = 9;
							u.append_id(gi->voxel_gi_get_sdf_texture(probe));
							uniforms.push_back(u);
						}
						{
							RD::Uniform u;
							u.uniform_type = RD::UNIFORM_TYPE_SAMPLER;
							u.binding = 10;
							u.append_id(material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED));
							uniforms.push_back(u);
						}

						if (plot) {
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 11;
								u.append_id(mipmaps[dmap.mipmap].texture);
								uniforms.push_back(u);
							}
						}

						dmap.uniform_set = RD::get_singleton()->uniform_set_create(
								uniforms,
								gi->voxel_gi_lighting_shader_version_shaders[(write && plot) ? VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_WRITE_PLOT : (write ? VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_WRITE : VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_PLOT)],
								0);
					}

					dynamic_maps.push_back(dmap);
				}
			}
		}

		last_probe_data_version = data_version;
		p_update_light_instances = true; //just in case

		RendererSceneRenderRD::get_singleton()->base_uniforms_changed();
	}

	// UDPDATE TIME

	if (has_dynamic_object_data) {
		//if it has dynamic object data, it needs to be cleared
		RD::get_singleton()->texture_clear(texture, Color(0, 0, 0, 0), 0, mipmaps.size(), 0, 1);
	}

	uint32_t light_count = 0;

	if (p_update_light_instances || p_dynamic_objects.size() > 0) {
		light_count = MIN(gi->voxel_gi_max_lights, (uint32_t)p_light_instances.size());

		{
			Transform3D to_cell = gi->voxel_gi_get_to_cell_xform(probe);
			Transform3D to_probe_xform = to_cell * transform.affine_inverse();

			//update lights

			for (uint32_t i = 0; i < light_count; i++) {
				VoxelGILight &l = gi->voxel_gi_lights[i];
				RID light_instance = p_light_instances[i];
				RID light = light_storage->light_instance_get_base_light(light_instance);

				l.type = RSG::light_storage->light_get_type(light);
				if (l.type == RS::LIGHT_DIRECTIONAL && RSG::light_storage->light_directional_get_sky_mode(light) == RS::LIGHT_DIRECTIONAL_SKY_MODE_SKY_ONLY) {
					light_count--;
					continue;
				}

				l.attenuation = RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_ATTENUATION);
				l.energy = RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_ENERGY) * RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_INDIRECT_ENERGY);

				if (RendererSceneRenderRD::get_singleton()->is_using_physical_light_units()) {
					l.energy *= RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_INTENSITY);

					l.energy *= gi->voxel_gi_get_baked_exposure_normalization(probe);

					// Convert from Luminous Power to Luminous Intensity
					if (l.type == RS::LIGHT_OMNI) {
						l.energy *= 1.0 / (Math_PI * 4.0);
					} else if (l.type == RS::LIGHT_SPOT) {
						// Spot Lights are not physically accurate, Luminous Intensity should change in relation to the cone angle.
						// We make this assumption to keep them easy to control.
						l.energy *= 1.0 / Math_PI;
					}
				}

				l.radius = to_cell.basis.xform(Vector3(RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_RANGE), 0, 0)).length();
				Color color = RSG::light_storage->light_get_color(light).srgb_to_linear();
				l.color[0] = color.r;
				l.color[1] = color.g;
				l.color[2] = color.b;

				l.cos_spot_angle = Math::cos(Math::deg_to_rad(RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_SPOT_ANGLE)));
				l.inv_spot_attenuation = 1.0f / RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_SPOT_ATTENUATION);

				Transform3D xform = light_storage->light_instance_get_base_transform(light_instance);

				Vector3 pos = to_probe_xform.xform(xform.origin);
				Vector3 dir = to_probe_xform.basis.xform(-xform.basis.get_column(2)).normalized();

				l.position[0] = pos.x;
				l.position[1] = pos.y;
				l.position[2] = pos.z;

				l.direction[0] = dir.x;
				l.direction[1] = dir.y;
				l.direction[2] = dir.z;

				l.has_shadow = RSG::light_storage->light_has_shadow(light);
			}

			RD::get_singleton()->buffer_update(gi->voxel_gi_lights_uniform, 0, sizeof(VoxelGILight) * light_count, gi->voxel_gi_lights);
		}
	}

	if (has_dynamic_object_data || p_update_light_instances || p_dynamic_objects.size()) {
		// PROCESS MIPMAPS
		if (mipmaps.size()) {
			//can update mipmaps

			Vector3i probe_size = gi->voxel_gi_get_octree_size(probe);

			VoxelGIPushConstant push_constant;

			push_constant.limits[0] = probe_size.x;
			push_constant.limits[1] = probe_size.y;
			push_constant.limits[2] = probe_size.z;
			push_constant.stack_size = mipmaps.size();
			push_constant.emission_scale = 1.0;
			push_constant.propagation = gi->voxel_gi_get_propagation(probe);
			push_constant.dynamic_range = gi->voxel_gi_get_dynamic_range(probe);
			push_constant.light_count = light_count;
			push_constant.aniso_strength = 0;

			/*		print_line("probe update to version " + itos(last_probe_version));
			print_line("propagation " + rtos(push_constant.propagation));
			print_line("dynrange " + rtos(push_constant.dynamic_range));
	*/
			RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();

			int passes;
			if (p_update_light_instances) {
				passes = gi->voxel_gi_is_using_two_bounces(probe) ? 2 : 1;
			} else {
				passes = 1; //only re-blitting is necessary
			}
			int wg_size = 64;
			int64_t wg_limit_x = (int64_t)RD::get_singleton()->limit_get(RD::LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_X);

			for (int pass = 0; pass < passes; pass++) {
				if (p_update_light_instances) {
					for (int i = 0; i < mipmaps.size(); i++) {
						if (i == 0) {
							RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[pass == 0 ? VOXEL_GI_SHADER_VERSION_COMPUTE_LIGHT : VOXEL_GI_SHADER_VERSION_COMPUTE_SECOND_BOUNCE]);
						} else if (i == 1) {
							RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[VOXEL_GI_SHADER_VERSION_COMPUTE_MIPMAP]);
						}

						if (pass == 1 || i > 0) {
							RD::get_singleton()->compute_list_add_barrier(compute_list); //wait til previous step is done
						}
						if (pass == 0 || i > 0) {
							RD::get_singleton()->compute_list_bind_uniform_set(compute_list, mipmaps[i].uniform_set, 0);
						} else {
							RD::get_singleton()->compute_list_bind_uniform_set(compute_list, mipmaps[i].second_bounce_uniform_set, 0);
						}

						push_constant.cell_offset = mipmaps[i].cell_offset;
						push_constant.cell_count = mipmaps[i].cell_count;

						int64_t wg_todo = (mipmaps[i].cell_count + wg_size - 1) / wg_size;
						while (wg_todo) {
							int64_t wg_count = MIN(wg_todo, wg_limit_x);
							RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(VoxelGIPushConstant));
							RD::get_singleton()->compute_list_dispatch(compute_list, wg_count, 1, 1);
							wg_todo -= wg_count;
							push_constant.cell_offset += wg_count * wg_size;
						}
					}

					RD::get_singleton()->compute_list_add_barrier(compute_list); //wait til previous step is done
				}

				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[VOXEL_GI_SHADER_VERSION_WRITE_TEXTURE]);

				for (int i = 0; i < mipmaps.size(); i++) {
					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, mipmaps[i].write_uniform_set, 0);

					push_constant.cell_offset = mipmaps[i].cell_offset;
					push_constant.cell_count = mipmaps[i].cell_count;

					int64_t wg_todo = (mipmaps[i].cell_count + wg_size - 1) / wg_size;
					while (wg_todo) {
						int64_t wg_count = MIN(wg_todo, wg_limit_x);
						RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(VoxelGIPushConstant));
						RD::get_singleton()->compute_list_dispatch(compute_list, wg_count, 1, 1);
						wg_todo -= wg_count;
						push_constant.cell_offset += wg_count * wg_size;
					}
				}
			}

			RD::get_singleton()->compute_list_end();
		}
	}

	has_dynamic_object_data = false; //clear until dynamic object data is used again

	if (p_dynamic_objects.size() && dynamic_maps.size()) {
		Vector3i octree_size = gi->voxel_gi_get_octree_size(probe);
		int multiplier = dynamic_maps[0].size / MAX(MAX(octree_size.x, octree_size.y), octree_size.z);

		Transform3D oversample_scale;
		oversample_scale.basis.scale(Vector3(multiplier, multiplier, multiplier));

		Transform3D to_cell = oversample_scale * gi->voxel_gi_get_to_cell_xform(probe);
		Transform3D to_world_xform = transform * to_cell.affine_inverse();
		Transform3D to_probe_xform = to_world_xform.affine_inverse();

		AABB probe_aabb(Vector3(), octree_size);

		//this could probably be better parallelized in compute..
		for (int i = 0; i < (int)p_dynamic_objects.size(); i++) {
			RenderGeometryInstance *instance = p_dynamic_objects[i];

			//transform aabb to voxel_gi
			AABB aabb = (to_probe_xform * instance->get_transform()).xform(instance->get_aabb());

			//this needs to wrap to grid resolution to avoid jitter
			//also extend margin a bit just in case
			Vector3i begin = aabb.position - Vector3i(1, 1, 1);
			Vector3i end = aabb.position + aabb.size + Vector3i(1, 1, 1);

			for (int j = 0; j < 3; j++) {
				if ((end[j] - begin[j]) & 1) {
					end[j]++; //for half extents split, it needs to be even
				}
				begin[j] = MAX(begin[j], 0);
				end[j] = MIN(end[j], octree_size[j] * multiplier);
			}

			//aabb = aabb.intersection(probe_aabb); //intersect
			aabb.position = begin;
			aabb.size = end - begin;

			//print_line("aabb: " + aabb);

			for (int j = 0; j < 6; j++) {
				//if (j != 0 && j != 3) {
				//	continue;
				//}
				static const Vector3 render_z[6] = {
					Vector3(1, 0, 0),
					Vector3(0, 1, 0),
					Vector3(0, 0, 1),
					Vector3(-1, 0, 0),
					Vector3(0, -1, 0),
					Vector3(0, 0, -1),
				};
				static const Vector3 render_up[6] = {
					Vector3(0, 1, 0),
					Vector3(0, 0, 1),
					Vector3(0, 1, 0),
					Vector3(0, 1, 0),
					Vector3(0, 0, 1),
					Vector3(0, 1, 0),
				};

				Vector3 render_dir = render_z[j];
				Vector3 up_dir = render_up[j];

				Vector3 center = aabb.get_center();
				Transform3D xform;
				xform.set_look_at(center - aabb.size * 0.5 * render_dir, center, up_dir);

				Vector3 x_dir = xform.basis.get_column(0).abs();
				int x_axis = int(Vector3(0, 1, 2).dot(x_dir));
				Vector3 y_dir = xform.basis.get_column(1).abs();
				int y_axis = int(Vector3(0, 1, 2).dot(y_dir));
				Vector3 z_dir = -xform.basis.get_column(2);
				int z_axis = int(Vector3(0, 1, 2).dot(z_dir.abs()));

				Rect2i rect(aabb.position[x_axis], aabb.position[y_axis], aabb.size[x_axis], aabb.size[y_axis]);
				bool x_flip = bool(Vector3(1, 1, 1).dot(xform.basis.get_column(0)) < 0);
				bool y_flip = bool(Vector3(1, 1, 1).dot(xform.basis.get_column(1)) < 0);
				bool z_flip = bool(Vector3(1, 1, 1).dot(xform.basis.get_column(2)) > 0);

				Projection cm;
				cm.set_orthogonal(-rect.size.width() / 2, rect.size.width() / 2, -rect.size.height() / 2, rect.size.height() / 2, 0.0001, aabb.size[z_axis]);

				if (RendererSceneRenderRD::get_singleton()->cull_argument.size() == 0) {
					RendererSceneRenderRD::get_singleton()->cull_argument.push_back(nullptr);
				}
				RendererSceneRenderRD::get_singleton()->cull_argument[0] = instance;

				float exposure_normalization = 1.0;
				if (RendererSceneRenderRD::get_singleton()->is_using_physical_light_units()) {
					exposure_normalization = gi->voxel_gi_get_baked_exposure_normalization(probe);
				}

				RendererSceneRenderRD::get_singleton()->_render_material(to_world_xform * xform, cm, true, RendererSceneRenderRD::get_singleton()->cull_argument, dynamic_maps[0].fb, Rect2i(Vector2i(), rect.size), exposure_normalization);

				VoxelGIDynamicPushConstant push_constant;
				memset(&push_constant, 0, sizeof(VoxelGIDynamicPushConstant));
				push_constant.limits[0] = octree_size.x;
				push_constant.limits[1] = octree_size.y;
				push_constant.limits[2] = octree_size.z;
				push_constant.light_count = p_light_instances.size();
				push_constant.x_dir[0] = x_dir[0];
				push_constant.x_dir[1] = x_dir[1];
				push_constant.x_dir[2] = x_dir[2];
				push_constant.y_dir[0] = y_dir[0];
				push_constant.y_dir[1] = y_dir[1];
				push_constant.y_dir[2] = y_dir[2];
				push_constant.z_dir[0] = z_dir[0];
				push_constant.z_dir[1] = z_dir[1];
				push_constant.z_dir[2] = z_dir[2];
				push_constant.z_base = xform.origin[z_axis];
				push_constant.z_sign = (z_flip ? -1.0 : 1.0);
				push_constant.pos_multiplier = float(1.0) / multiplier;
				push_constant.dynamic_range = gi->voxel_gi_get_dynamic_range(probe);
				push_constant.flip_x = x_flip;
				push_constant.flip_y = y_flip;
				push_constant.rect_pos[0] = rect.position[0];
				push_constant.rect_pos[1] = rect.position[1];
				push_constant.rect_size[0] = rect.size[0];
				push_constant.rect_size[1] = rect.size[1];
				push_constant.prev_rect_ofs[0] = 0;
				push_constant.prev_rect_ofs[1] = 0;
				push_constant.prev_rect_size[0] = 0;
				push_constant.prev_rect_size[1] = 0;
				push_constant.on_mipmap = false;
				push_constant.propagation = gi->voxel_gi_get_propagation(probe);
				push_constant.pad[0] = 0;
				push_constant.pad[1] = 0;
				push_constant.pad[2] = 0;

				//process lighting
				RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[VOXEL_GI_SHADER_VERSION_DYNAMIC_OBJECT_LIGHTING]);
				RD::get_singleton()->compute_list_bind_uniform_set(compute_list, dynamic_maps[0].uniform_set, 0);
				RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(VoxelGIDynamicPushConstant));
				RD::get_singleton()->compute_list_dispatch(compute_list, Math::division_round_up(rect.size.x, 8), Math::division_round_up(rect.size.y, 8), 1);
				//print_line("rect: " + itos(i) + ": " + rect);

				for (int k = 1; k < dynamic_maps.size(); k++) {
					// enlarge the rect if needed so all pixels fit when downscaled,
					// this ensures downsampling is smooth and optimal because no pixels are left behind

					//x
					if (rect.position.x & 1) {
						rect.size.x++;
						push_constant.prev_rect_ofs[0] = 1; //this is used to ensure reading is also optimal
					} else {
						push_constant.prev_rect_ofs[0] = 0;
					}
					if (rect.size.x & 1) {
						rect.size.x++;
					}

					rect.position.x >>= 1;
					rect.size.x = MAX(1, rect.size.x >> 1);

					//y
					if (rect.position.y & 1) {
						rect.size.y++;
						push_constant.prev_rect_ofs[1] = 1;
					} else {
						push_constant.prev_rect_ofs[1] = 0;
					}
					if (rect.size.y & 1) {
						rect.size.y++;
					}

					rect.position.y >>= 1;
					rect.size.y = MAX(1, rect.size.y >> 1);

					//shrink limits to ensure plot does not go outside map
					if (dynamic_maps[k].mipmap > 0) {
						for (int l = 0; l < 3; l++) {
							push_constant.limits[l] = MAX(1, push_constant.limits[l] >> 1);
						}
					}

					//print_line("rect: " + itos(i) + ": " + rect);
					push_constant.rect_pos[0] = rect.position[0];
					push_constant.rect_pos[1] = rect.position[1];
					push_constant.prev_rect_size[0] = push_constant.rect_size[0];
					push_constant.prev_rect_size[1] = push_constant.rect_size[1];
					push_constant.rect_size[0] = rect.size[0];
					push_constant.rect_size[1] = rect.size[1];
					push_constant.on_mipmap = dynamic_maps[k].mipmap > 0;

					RD::get_singleton()->compute_list_add_barrier(compute_list);

					if (dynamic_maps[k].mipmap < 0) {
						RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_WRITE]);
					} else if (k < dynamic_maps.size() - 1) {
						RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_WRITE_PLOT]);
					} else {
						RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_PLOT]);
					}
					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, dynamic_maps[k].uniform_set, 0);
					RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(VoxelGIDynamicPushConstant));
					RD::get_singleton()->compute_list_dispatch(compute_list, Math::division_round_up(rect.size.x, 8), Math::division_round_up(rect.size.y, 8), 1);
				}

				RD::get_singleton()->compute_list_end();
			}
		}

		has_dynamic_object_data = true; //clear until dynamic object data is used again
	}

	last_probe_version = gi->voxel_gi_get_version(probe);
}

void GI::voxel_gi_initialize(RID p_voxel_gi) {
	voxel_gi_owner.initialize_rid(p_voxel_gi, VoxelGI());
}

void GI::process_gi(Ref<RenderSceneBuffersRD> p_render_buffers, const RID *p_normal_roughness_slices, RID p_voxel_gi_buffer, RID p_environment, uint32_t p_view_count, const Projection *p_projections, const Vector3 *p_eye_offsets, const Transform3D &p_cam_transform, const PagedArray<RID> &p_voxel_gi_instances) {
	RendererRD::TextureStorage *texture_storage = RendererRD::TextureStorage::get_singleton();
	RendererRD::MaterialStorage *material_storage = RendererRD::MaterialStorage::get_singleton();

	ERR_FAIL_COND_MSG(p_view_count > 2, "Maximum of 2 views supported for Processing GI.");

	RD::get_singleton()->draw_command_begin_label("GI Render");

	ERR_FAIL_COND(p_render_buffers.is_null());

	Ref<RenderBuffersGI> rbgi = p_render_buffers->get_custom_data(RB_SCOPE_GI);
	ERR_FAIL_COND(rbgi.is_null());

	Size2i internal_size = p_render_buffers->get_internal_size();

	if (rbgi->using_half_size_gi != half_resolution) {
		p_render_buffers->clear_context(RB_SCOPE_GI);
	}

	if (!p_render_buffers->has_texture(RB_SCOPE_GI, RB_TEX_AMBIENT)) {
		Size2i size = internal_size;
		uint32_t usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT;

		if (half_resolution) {
			size.x >>= 1;
			size.y >>= 1;
		}

		p_render_buffers->create_texture(RB_SCOPE_GI, RB_TEX_AMBIENT, RD::DATA_FORMAT_R16G16B16A16_SFLOAT, usage_bits, RD::TEXTURE_SAMPLES_1, size);
		p_render_buffers->create_texture(RB_SCOPE_GI, RB_TEX_REFLECTION, RD::DATA_FORMAT_R16G16B16A16_SFLOAT, usage_bits, RD::TEXTURE_SAMPLES_1, size);

		rbgi->using_half_size_gi = half_resolution;
	}

	// Setup our scene data
	{
		SceneData scene_data;

		if (rbgi->scene_data_ubo.is_null()) {
			rbgi->scene_data_ubo = RD::get_singleton()->uniform_buffer_create(sizeof(SceneData));
		}

		Projection correction;
		correction.set_depth_correction(false);

		for (uint32_t v = 0; v < p_view_count; v++) {
			Projection temp = correction * p_projections[v];

			RendererRD::MaterialStorage::store_camera(temp.inverse(), scene_data.inv_projection[v]);
			scene_data.eye_offset[v][0] = p_eye_offsets[v].x;
			scene_data.eye_offset[v][1] = p_eye_offsets[v].y;
			scene_data.eye_offset[v][2] = p_eye_offsets[v].z;
			scene_data.eye_offset[v][3] = 0.0;
		}

		// Note that we will be ignoring the origin of this transform.
		RendererRD::MaterialStorage::store_transform(p_cam_transform, scene_data.cam_transform);

		scene_data.screen_size[0] = internal_size.x;
		scene_data.screen_size[1] = internal_size.y;

		RD::get_singleton()->buffer_update(rbgi->scene_data_ubo, 0, sizeof(SceneData), &scene_data);
	}

	// Now compute the contents of our buffers.
	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();

	// Render each eye separately.
	// We need to look into whether we can make our compute shader use Multiview but not sure that works or makes a difference..

	// setup our push constant

	PushConstant push_constant;

	push_constant.max_voxel_gi_instances = MIN((uint64_t)MAX_VOXEL_GI_INSTANCES, p_voxel_gi_instances.size());
	push_constant.high_quality_vct = voxel_gi_quality == RS::VOXEL_GI_QUALITY_HIGH;

	// these should be the same for all views
	push_constant.orthogonal = p_projections[0].is_orthogonal();
	push_constant.z_near = p_projections[0].get_z_near();
	push_constant.z_far = p_projections[0].get_z_far();

	// these are only used if we have 1 view, else we use the projections in our scene data
	push_constant.proj_info[0] = -2.0f / (internal_size.x * p_projections[0].columns[0][0]);
	push_constant.proj_info[1] = -2.0f / (internal_size.y * p_projections[0].columns[1][1]);
	push_constant.proj_info[2] = (1.0f - p_projections[0].columns[0][2]) / p_projections[0].columns[0][0];
	push_constant.proj_info[3] = (1.0f + p_projections[0].columns[1][2]) / p_projections[0].columns[1][1];

	bool use_sdfgi = p_render_buffers->has_custom_data(RB_SCOPE_SDFGI);
	bool use_voxel_gi_instances = push_constant.max_voxel_gi_instances > 0;

	Ref<SDFGI> sdfgi;
	if (use_sdfgi) {
		sdfgi = p_render_buffers->get_custom_data(RB_SCOPE_SDFGI);
	}

	uint32_t pipeline_specialization = 0;
	if (rbgi->using_half_size_gi) {
		pipeline_specialization |= SHADER_SPECIALIZATION_HALF_RES;
	}
	if (p_view_count > 1) {
		pipeline_specialization |= SHADER_SPECIALIZATION_USE_FULL_PROJECTION_MATRIX;
	}
	bool has_vrs_texture = p_render_buffers->has_texture(RB_SCOPE_VRS, RB_TEXTURE);
	if (has_vrs_texture) {
		pipeline_specialization |= SHADER_SPECIALIZATION_USE_VRS;
	}

	Mode mode = (use_sdfgi && use_voxel_gi_instances) ? MODE_COMBINED : (use_sdfgi ? MODE_SDFGI : MODE_VOXEL_GI);

	// for (uint32_t v = 0; v < p_view_count; v++) {
	// 	push_constant.view_index = v;

	// 	// setup our uniform set
	// 	if (rbgi->uniform_set[v].is_null() || !RD::get_singleton()->uniform_set_is_valid(rbgi->uniform_set[v])) {
	// 		Vector<RD::Uniform> uniforms;
	// 		{
	// 			RD::Uniform u;
	// 			u.binding = 1;
	// 			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
	// 			for (uint32_t j = 0; j < SDFGI::MAX_CASCADES; j++) {
	// 				if (use_sdfgi && j < sdfgi->cascades.size()) {
	// 					u.append_id(sdfgi->cascades[j].sdf_tex);
	// 				} else {
	// 					u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_3D_WHITE));
	// 				}
	// 			}
	// 			uniforms.push_back(u);
	// 		}
	// 		{ // 0x000002817f4a8ca8 {alloc={chunks=0x000002810d821ed0 {0x000002810d65b9e0 {...}} free_list_chunks=0x000002810afa4a70 {...} ...} }
	// 			RD::Uniform u;
	// 			u.binding = 2;
	// 			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
	// 			for (uint32_t j = 0; j < SDFGI::MAX_CASCADES; j++) {
	// 				if (use_sdfgi && j < sdfgi->cascades.size()) {
	// 					u.append_id(sdfgi->cascades[j].light_tex);
	// 				} else {
	// 					u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_3D_WHITE));
	// 				}
	// 			}
	// 			uniforms.push_back(u);
	// 		}
	// 		{
	// 			RD::Uniform u;
	// 			u.binding = 3;
	// 			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
	// 			for (uint32_t j = 0; j < SDFGI::MAX_CASCADES; j++) {
	// 				if (use_sdfgi && j < sdfgi->cascades.size()) {
	// 					u.append_id(sdfgi->cascades[j].light_aniso_0_tex);
	// 				} else {
	// 					u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_3D_WHITE));
	// 				}
	// 			}
	// 			uniforms.push_back(u);
	// 		}
	// 		{
	// 			RD::Uniform u;
	// 			u.binding = 4;
	// 			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
	// 			for (uint32_t j = 0; j < SDFGI::MAX_CASCADES; j++) {
	// 				if (use_sdfgi && j < sdfgi->cascades.size()) {
	// 					u.append_id(sdfgi->cascades[j].light_aniso_1_tex);
	// 				} else {
	// 					u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_3D_WHITE));
	// 				}
	// 			}
	// 			uniforms.push_back(u);
	// 		}
	// 		{
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
	// 			u.binding = 5;
	// 			if (use_sdfgi) {
	// 				u.append_id(sdfgi->occlusion_texture);
	// 			} else {
	// 				u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_3D_WHITE));
	// 			}
	// 			uniforms.push_back(u);
	// 		}
	// 		{
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_SAMPLER;
	// 			u.binding = 6;
	// 			u.append_id(material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED));
	// 			uniforms.push_back(u);
	// 		}
	// 		{
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_SAMPLER;
	// 			u.binding = 7;
	// 			u.append_id(material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED));
	// 			uniforms.push_back(u);
	// 		}
	// 		{
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
	// 			u.binding = 9;
	// 			u.append_id(p_render_buffers->get_texture_slice(RB_SCOPE_GI, RB_TEX_AMBIENT, v, 0));
	// 			uniforms.push_back(u);
	// 		}

	// 		{
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
	// 			u.binding = 10;
	// 			u.append_id(p_render_buffers->get_texture_slice(RB_SCOPE_GI, RB_TEX_REFLECTION, v, 0));
	// 			uniforms.push_back(u);
	// 		}

	// 		{
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
	// 			u.binding = 11;
	// 			if (use_sdfgi) {
	// 				u.append_id(sdfgi->lightprobe_texture);
	// 			} else {
	// 				u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_2D_ARRAY_WHITE));
	// 			}
	// 			uniforms.push_back(u);
	// 		}
	// 		{
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
	// 			u.binding = 12;
	// 			u.append_id(p_render_buffers->get_depth_texture(v));
	// 			uniforms.push_back(u);
	// 		}
	// 		{
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
	// 			u.binding = 13;
	// 			u.append_id(p_normal_roughness_slices[v]);
	// 			uniforms.push_back(u);
	// 		}
	// 		{
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
	// 			u.binding = 14;
	// 			RID buffer = p_voxel_gi_buffer.is_valid() ? p_voxel_gi_buffer : texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_BLACK);
	// 			u.append_id(buffer);
	// 			uniforms.push_back(u);
	// 		}
	// 		{
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
	// 			u.binding = 15;
	// 			u.append_id(sdfgi_ubo);
	// 			uniforms.push_back(u);
	// 		}
	// 		{
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
	// 			u.binding = 16;
	// 			u.append_id(rbgi->get_voxel_gi_buffer());
	// 			uniforms.push_back(u);
	// 		}
	// 		{
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
	// 			u.binding = 17;
	// 			for (int i = 0; i < MAX_VOXEL_GI_INSTANCES; i++) {
	// 				u.append_id(rbgi->voxel_gi_textures[i]);
	// 			}
	// 			uniforms.push_back(u);
	// 		}
	// 		{
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
	// 			u.binding = 18;
	// 			u.append_id(rbgi->scene_data_ubo);
	// 			uniforms.push_back(u);
	// 		}
	// 		if (RendererSceneRenderRD::get_singleton()->is_vrs_supported()) {
	// 			RD::Uniform u;
	// 			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
	// 			u.binding = 19;
	// 			RID buffer = has_vrs_texture ? p_render_buffers->get_texture_slice(RB_SCOPE_VRS, RB_TEXTURE, v, 0) : texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_VRS);
	// 			u.append_id(buffer);
	// 			uniforms.push_back(u);
	// 		}

	// 		rbgi->uniform_set[v] = RD::get_singleton()->uniform_set_create(uniforms, shader.version_get_shader(shader_version, 0), 0);
	// 	}

	// 	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, pipelines[pipeline_specialization][mode]);
	// 	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, rbgi->uniform_set[v], 0);
	// 	RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(PushConstant));

	// 	if (rbgi->using_half_size_gi) {
	// 		RD::get_singleton()->compute_list_dispatch_threads(compute_list, internal_size.x >> 1, internal_size.y >> 1, 1);
	// 	} else {
	// 		RD::get_singleton()->compute_list_dispatch_threads(compute_list, internal_size.x, internal_size.y, 1);
	// 	}
	// }

	RD::get_singleton()->compute_list_end();
	RD::get_singleton()->draw_command_end_label();
}

void GI::VoxelGIInstance::debug(RD::DrawListID p_draw_list, RID p_framebuffer, const Projection &p_camera_with_transform, bool p_lighting, bool p_emission, float p_alpha) {
	// RendererRD::MaterialStorage *material_storage = RendererRD::MaterialStorage::get_singleton();

	// if (mipmaps.size() == 0) {
	// 	return;
	// }

	// Projection cam_transform = (p_camera_with_transform * Projection(transform)) * Projection(gi->voxel_gi_get_to_cell_xform(probe).affine_inverse());

	// int level = 0;
	// Vector3i octree_size = gi->voxel_gi_get_octree_size(probe);

	// VoxelGIDebugPushConstant push_constant;
	// push_constant.alpha = p_alpha;
	// push_constant.dynamic_range = gi->voxel_gi_get_dynamic_range(probe);
	// push_constant.cell_offset = mipmaps[level].cell_offset;
	// push_constant.level = level;

	// push_constant.bounds[0] = octree_size.x >> level;
	// push_constant.bounds[1] = octree_size.y >> level;
	// push_constant.bounds[2] = octree_size.z >> level;
	// push_constant.pad = 0;

	// for (int i = 0; i < 4; i++) {
	// 	for (int j = 0; j < 4; j++) {
	// 		push_constant.projection[i * 4 + j] = cam_transform.columns[i][j];
	// 	}
	// }

	// if (gi->voxel_gi_debug_uniform_set.is_valid()) {
	// 	RD::get_singleton()->free(gi->voxel_gi_debug_uniform_set);
	// }
	// Vector<RD::Uniform> uniforms;
	// {
	// 	RD::Uniform u;
	// 	u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
	// 	u.binding = 1;
	// 	u.append_id(gi->voxel_gi_get_data_buffer(probe));
	// 	uniforms.push_back(u);
	// }
	// {
	// 	RD::Uniform u;
	// 	u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
	// 	u.binding = 2;
	// 	u.append_id(texture);
	// 	uniforms.push_back(u);
	// }
	// {
	// 	RD::Uniform u;
	// 	u.uniform_type = RD::UNIFORM_TYPE_SAMPLER;
	// 	u.binding = 3;
	// 	u.append_id(material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED));
	// 	uniforms.push_back(u);
	// }

	// int cell_count;
	// if (!p_emission && p_lighting && has_dynamic_object_data) {
	// 	cell_count = push_constant.bounds[0] * push_constant.bounds[1] * push_constant.bounds[2];
	// } else {
	// 	cell_count = mipmaps[level].cell_count;
	// }

	// gi->voxel_gi_debug_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->voxel_gi_debug_shader_version_shaders[0], 0);

	// int voxel_gi_debug_pipeline = VOXEL_GI_DEBUG_COLOR;
	// if (p_emission) {
	// 	voxel_gi_debug_pipeline = VOXEL_GI_DEBUG_EMISSION;
	// } else if (p_lighting) {
	// 	voxel_gi_debug_pipeline = has_dynamic_object_data ? VOXEL_GI_DEBUG_LIGHT_FULL : VOXEL_GI_DEBUG_LIGHT;
	// }
	// RD::get_singleton()->draw_list_bind_render_pipeline(
	// 		p_draw_list,
	// 		gi->voxel_gi_debug_shader_version_pipelines[voxel_gi_debug_pipeline].get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(p_framebuffer)));
	// RD::get_singleton()->draw_list_bind_uniform_set(p_draw_list, gi->voxel_gi_debug_uniform_set, 0);
	// RD::get_singleton()->draw_list_set_push_constant(p_draw_list, &push_constant, sizeof(VoxelGIDebugPushConstant));
	// RD::get_singleton()->draw_list_draw(p_draw_list, false, cell_count, 36);
}

RID GI::RenderBuffersGI::get_voxel_gi_buffer() {
	if (voxel_gi_buffer.is_null()) {
		voxel_gi_buffer = RD::get_singleton()->uniform_buffer_create(sizeof(GI::VoxelGIData) * GI::MAX_VOXEL_GI_INSTANCES);
	}
	return voxel_gi_buffer;
}

void GI::RenderBuffersGI::free_data() {
	for (uint32_t v = 0; v < RendererSceneRender::MAX_RENDER_VIEWS; v++) {
		if (RD::get_singleton()->uniform_set_is_valid(uniform_set[v])) {
			RD::get_singleton()->free(uniform_set[v]);
		}
		uniform_set[v] = RID();
	}

	if (scene_data_ubo.is_valid()) {
		RD::get_singleton()->free(scene_data_ubo);
		scene_data_ubo = RID();
	}

	if (voxel_gi_buffer.is_valid()) {
		RD::get_singleton()->free(voxel_gi_buffer);
		voxel_gi_buffer = RID();
	}
}

}  // namespace lain::RendererRD