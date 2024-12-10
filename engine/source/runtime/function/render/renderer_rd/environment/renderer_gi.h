#ifndef RENDERER_GI_RD_H
#define RENDERER_GI_RD_H
#include "function/render/rendering_system/renderer_gi_api.h"

namespace lain::RendererRD{
  class GI : public RendererGI {
 private:
  static GI* p_singleton;
  public:
  	enum {
		MAX_VOXEL_GI_INSTANCES = 8
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

	// class SDFGI : public RenderBufferCustomDataRD{
	// 	LCLASS(SDFGI, RenderBufferCustomDataRD);
	// 	enum {
	// 		MAX_CASCADES = 8,
	// 		CASCADE_SIZE = 128,
	// 		PROBE_DIVISOR = 16,
	// 		ANISOTROPY_SIZE = 6,
	// 		MAX_DYNAMIC_LIGHTS = 128,
	// 		MAX_STATIC_LIGHTS = 1024,
	// 		LIGHTPROBE_OCT_SIZE = 6,
	// 		SH_SIZE = 16
	// 	};
	// 	GI *gi = nullptr;

	// };
	class SDFGI{
	public:
		enum{
			MAX_CASCADES = 8,
		};
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


  ~GI(){
	p_singleton = this;
	}
  GI(){
	p_singleton = nullptr;
	}
  static GI* get_singleton() { return p_singleton;}
  bool owns_voxel_gi(RID p_rid) {return false;}
	int sdfgi_get_lightprobe_octahedron_size() const { return 1; }
  void init();
  RID default_voxel_gi_buffer;
  };
}
#endif