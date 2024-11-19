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
  ~GI(){}
  GI(){}
  static GI* get_singleton() { return p_singleton;}
  bool owns_voxel_gi(RID p_rid) {return false;}
	int sdfgi_get_lightprobe_octahedron_size() const { return 1; }
  void init();
  RID default_voxel_gi_buffer;
  };
}
#endif