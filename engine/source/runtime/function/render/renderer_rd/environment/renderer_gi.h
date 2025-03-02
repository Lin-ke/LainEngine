#ifndef RENDERER_GI_RD_H
#define RENDERER_GI_RD_H
#include "function/render/rendering_system/renderer_gi_api.h"
#include "function/render/renderer_rd/storage/render_buffer_custom_data_rd.h"
#include "function/render/rendering_system/utilities.h"
#include "../shaders/voxel_gi.glsl.gen.h"
#include "core/templates/paged_array.h"
#include "function/render/scene/renderer_geometry_instance_api.h"
namespace lain::RendererRD{
  class GI : public RendererGI {
 private:
  static GI* p_singleton;
  public:
  	enum {
		MAX_VOXEL_GI_INSTANCES = 8
	};

	struct VoxelGI {
		RID octree_buffer;
		RID data_buffer;
		RID sdf_texture;

		uint32_t octree_buffer_size = 0;
		uint32_t data_buffer_size = 0;

		Vector<int> level_counts;

		int cell_count = 0;

		Transform3D to_cell_xform;
		AABB bounds;
		Vector3i octree_size;

		float dynamic_range = 2.0;
		float energy = 1.0;
		float baked_exposure = 1.0;
		float bias = 1.4;
		float normal_bias = 0.0;
		float propagation = 0.5;
		bool interior = false;
		bool use_two_bounces = true;

		uint32_t version = 1;
		uint32_t data_version = 1;

		Dependency dependency;
	};



	struct VoxelGIData {
		float xform[16]; // 64 - 64

		float bounds[3]; // 12 - 76
		float dynamic_range; // 4 - 80

		float bias; // 4 - 84
		float normal_bias; // 4 - 88
		uint32_t blend_ambient; // 4 - 92
		uint32_t mipmaps; // 4 - 96

		float pad[3]; // 12 - 108
		float exposure_normalization; // 4 - 112
	};
	RID sdfgi_ubo;

	class SDFGI : public RenderBufferCustomDataRD{
		LCLASS(SDFGI, RenderBufferCustomDataRD);
		enum {
			MAX_CASCADES = 8,
			CASCADE_SIZE = 128,
			PROBE_DIVISOR = 16,
			ANISOTROPY_SIZE = 6,
			MAX_DYNAMIC_LIGHTS = 128,
			MAX_STATIC_LIGHTS = 1024,
			LIGHTPROBE_OCT_SIZE = 6,
			SH_SIZE = 16
		};
		GI *gi = nullptr;

	};


	struct SDFGIData {
		float grid_size[3];
		uint32_t max_cascades;

		uint32_t use_occlusion;
		int32_t probe_axis_size;
		float probe_to_uvw;
		float normal_bias;

		float lightprobe_tex_pixel_size[3];
		float energy;

		float lightprobe_uv_offset[3];
		float y_mult;

		float occlusion_clamp[3];
		uint32_t pad3;

		float occlusion_renormalize[3];
		uint32_t pad4;

		float cascade_probe_size[3];
		uint32_t pad5;

		struct ProbeCascadeData {
			float position[3]; //offset of (0,0,0) in world coordinates
			float to_probe; // 1/bounds * grid_size
			int32_t probe_world_offset[3];
			float to_cell; // 1/bounds * grid_size
			float pad[3];
			float exposure_normalization;
		};

		ProbeCascadeData cascades[SDFGI::MAX_CASCADES];
	};

	virtual void sdfgi_reset() override;
	uint32_t sdfgi_current_version = 0;

	/// GI class 
  ~GI(){
	p_singleton = nullptr;
	}
  GI(){
	p_singleton = this;
	}
  static GI* get_singleton() { return p_singleton;}

	mutable RID_Owner<VoxelGI, true> voxel_gi_owner;
	// 下面有个instance owner
	
	
	int sdfgi_get_lightprobe_octahedron_size() const { return 1; }
  void init();
	
	
	/// Voxel GI
  bool owns_voxel_gi(RID p_rid) {return voxel_gi_owner.owns(p_rid);}
	RID default_voxel_gi_buffer;

	virtual RID voxel_gi_allocate() override;
	virtual void voxel_gi_free(RID p_voxel_gi) override;
	virtual void voxel_gi_initialize(RID p_voxel_gi) override;

	virtual void voxel_gi_allocate_data(RID p_voxel_gi, const Transform3D &p_to_cell_xform, const AABB &p_aabb, const Vector3i &p_octree_size, const Vector<uint8_t> &p_octree_cells, const Vector<uint8_t> &p_data_cells, const Vector<uint8_t> &p_distance_field, const Vector<int> &p_level_counts) override;

	virtual AABB voxel_gi_get_bounds(RID p_voxel_gi) const override;
	virtual Vector3i voxel_gi_get_octree_size(RID p_voxel_gi) const override;
	virtual Vector<uint8_t> voxel_gi_get_octree_cells(RID p_voxel_gi) const override;
	virtual Vector<uint8_t> voxel_gi_get_data_cells(RID p_voxel_gi) const override;
	virtual Vector<uint8_t> voxel_gi_get_distance_field(RID p_voxel_gi) const override;

	virtual Vector<int> voxel_gi_get_level_counts(RID p_voxel_gi) const override;
	virtual Transform3D voxel_gi_get_to_cell_xform(RID p_voxel_gi) const override;

	virtual void voxel_gi_set_dynamic_range(RID p_voxel_gi, float p_range) override;
	virtual float voxel_gi_get_dynamic_range(RID p_voxel_gi) const override;

	virtual void voxel_gi_set_propagation(RID p_voxel_gi, float p_range) override;
	virtual float voxel_gi_get_propagation(RID p_voxel_gi) const override;

	virtual void voxel_gi_set_energy(RID p_voxel_gi, float p_energy) override;
	virtual float voxel_gi_get_energy(RID p_voxel_gi) const override;

	virtual void voxel_gi_set_baked_exposure_normalization(RID p_voxel_gi, float p_baked_exposure) override;
	virtual float voxel_gi_get_baked_exposure_normalization(RID p_voxel_gi) const override;

	virtual void voxel_gi_set_bias(RID p_voxel_gi, float p_bias) override;
	virtual float voxel_gi_get_bias(RID p_voxel_gi) const override;

	virtual void voxel_gi_set_normal_bias(RID p_voxel_gi, float p_range) override;
	virtual float voxel_gi_get_normal_bias(RID p_voxel_gi) const override;

	virtual void voxel_gi_set_interior(RID p_voxel_gi, bool p_enable) override;
	virtual bool voxel_gi_is_interior(RID p_voxel_gi) const override;

	virtual void voxel_gi_set_use_two_bounces(RID p_voxel_gi, bool p_enable) override;
	virtual bool voxel_gi_is_using_two_bounces(RID p_voxel_gi) const override;

	virtual uint32_t voxel_gi_get_version(RID p_probe) const override;
	uint32_t voxel_gi_get_data_version(RID p_probe);

	RID voxel_gi_get_octree_buffer(RID p_voxel_gi) const;
	RID voxel_gi_get_data_buffer(RID p_voxel_gi) const;

	RID voxel_gi_get_sdf_texture(RID p_voxel_gi);

	Dependency *voxel_gi_get_dependency(RID p_voxel_gi) const;


	
	/* VOXEL_GI INSTANCE */
	struct VoxelGIInstance {
		// access to our containers
		GI *gi = nullptr;

		RID probe;
		RID texture;
		RID write_buffer;

		struct Mipmap {
			RID texture;
			RID uniform_set;
			RID second_bounce_uniform_set;
			RID write_uniform_set;
			uint32_t level;
			uint32_t cell_offset;
			uint32_t cell_count;
		};
		Vector<Mipmap> mipmaps;

		struct DynamicMap {
			RID texture; //color normally, or emission on first pass
			RID fb_depth; //actual depth buffer for the first pass, float depth for later passes
			RID depth; //actual depth buffer for the first pass, float depth for later passes
			RID normal; //normal buffer for the first pass
			RID albedo; //emission buffer for the first pass
			RID orm; //orm buffer for the first pass
			RID fb; //used for rendering, only valid on first map
			RID uniform_set;
			uint32_t size;
			int mipmap; // mipmap to write to, -1 if no mipmap assigned
		};

		Vector<DynamicMap> dynamic_maps;

		int slot = -1;
		uint32_t last_probe_version = 0;
		uint32_t last_probe_data_version = 0;

		//uint64_t last_pass = 0;
		uint32_t render_index = 0;

		bool has_dynamic_object_data = false;

		Transform3D transform;

		void update(bool p_update_light_instances, const Vector<RID> &p_light_instances, const PagedArray<RenderGeometryInstance *> &p_dynamic_objects);
		void debug(RD::DrawListID p_draw_list, RID p_framebuffer, const Projection &p_camera_with_transform, bool p_lighting, bool p_emission, float p_alpha);
		void free_resources();
	};
	mutable RID_Owner<VoxelGIInstance> voxel_gi_instance_owner;

	
	RID voxel_gi_instance_create(RID p_base);
	void voxel_gi_instance_set_transform_to_data(RID p_probe, const Transform3D &p_xform);
	bool voxel_gi_needs_update(RID p_probe) const;
	void voxel_gi_update(RID p_probe, bool p_update_light_instances, const Vector<RID> &p_light_instances, const PagedArray<RenderGeometryInstance *> &p_dynamic_objects);
	void debug_voxel_gi(RID p_voxel_gi, RD::DrawListID p_draw_list, RID p_framebuffer, const Projection &p_camera_with_transform, bool p_lighting, bool p_emission, float p_alpha);

	
	_FORCE_INLINE_ RID voxel_gi_instance_get_texture(RID p_probe) {
		VoxelGIInstance *voxel_gi = voxel_gi_instance_owner.get_or_null(p_probe);
		ERR_FAIL_NULL_V(voxel_gi, RID());
		return voxel_gi->texture;
	};

	_FORCE_INLINE_ void voxel_gi_instance_set_render_index(RID p_probe, uint32_t p_index) {
		VoxelGIInstance *voxel_gi = voxel_gi_instance_owner.get_or_null(p_probe);
		ERR_FAIL_NULL(voxel_gi);

		voxel_gi->render_index = p_index;
	};

	bool voxel_gi_instance_owns(RID p_rid) const {
		return voxel_gi_instance_owner.owns(p_rid);
	}

	void voxel_gi_instance_free(RID p_rid);

	RS::VoxelGIQuality voxel_gi_quality = RS::VOXEL_GI_QUALITY_LOW;


	
	struct VoxelGILight {
		uint32_t type;
		float energy;
		float radius;
		float attenuation;

		float color[3];
		float cos_spot_angle;

		float position[3];
		float inv_spot_attenuation;

		float direction[3];
		uint32_t has_shadow;
	};

	struct VoxelGIPushConstant {
		int32_t limits[3];
		uint32_t stack_size;

		float emission_scale;
		float propagation;
		float dynamic_range;
		uint32_t light_count;

		uint32_t cell_offset;
		uint32_t cell_count;
		float aniso_strength;
		uint32_t pad;
	};

	struct VoxelGIDynamicPushConstant {
		int32_t limits[3];
		uint32_t light_count;
		int32_t x_dir[3];
		float z_base;
		int32_t y_dir[3];
		float z_sign;
		int32_t z_dir[3];
		float pos_multiplier;
		uint32_t rect_pos[2];
		uint32_t rect_size[2];
		uint32_t prev_rect_ofs[2];
		uint32_t prev_rect_size[2];
		uint32_t flip_x;
		uint32_t flip_y;
		float dynamic_range;
		uint32_t on_mipmap;
		float propagation;
		float pad[3];
	};

	VoxelGILight *voxel_gi_lights = nullptr;
	uint32_t voxel_gi_max_lights = 32;
	RID voxel_gi_lights_uniform;

	enum {
		VOXEL_GI_SHADER_VERSION_COMPUTE_LIGHT,
		VOXEL_GI_SHADER_VERSION_COMPUTE_SECOND_BOUNCE,
		VOXEL_GI_SHADER_VERSION_COMPUTE_MIPMAP,
		VOXEL_GI_SHADER_VERSION_WRITE_TEXTURE,
		VOXEL_GI_SHADER_VERSION_DYNAMIC_OBJECT_LIGHTING,
		VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_WRITE,
		VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_PLOT,
		VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_WRITE_PLOT,
		VOXEL_GI_SHADER_VERSION_MAX
	};

	VoxelGiShaderRD voxel_gi_shader;
	RID voxel_gi_lighting_shader_version;
	RID voxel_gi_lighting_shader_version_shaders[VOXEL_GI_SHADER_VERSION_MAX];
	RID voxel_gi_lighting_shader_version_pipelines[VOXEL_GI_SHADER_VERSION_MAX];

};
}
#endif