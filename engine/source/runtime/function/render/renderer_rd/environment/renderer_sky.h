#ifndef RENDERER_SkyRD_RD_H
#define RENDERER_SkyRD_RD_H
#include "../pipeline_cache_rd.h"
#include "core/io/rid_owner.h"
#include "function/render/renderer_rd/shaders/sky.glsl.gen.h"
#include "function/render/rendering_device/rendering_device.h"
#include "function/render/scene/renderer_scene_renderer_api.h"
#include "function/shader/shader_compiler.h"
#include "../storage/material_storage.h"
namespace lain::RendererRD {
class SkyRD {
 private:
  // Skys need less info from Directional Lights than the normal shaders
  struct SkyDirectionalLightData {
    float direction[3];
    float energy;
    float color[3];
    float size;
    uint32_t enabled;
    uint32_t pad[3];
  };
  

 public:
  ~SkyRD();
  SkyRD();
  float sky_get_baked_exposure(RID p_sky) const;
  int roughness_layers;
  bool sky_use_cubemap_array; // default true;
  struct Sky {
    float baked_exposure = 1.0;
  };
  mutable RID_Owner<Sky, true> sky_owner;
  void init();

  enum SkyVersion {
    SKY_VERSION_BACKGROUND,
    SKY_VERSION_HALF_RES,
    SKY_VERSION_QUARTER_RES,
    SKY_VERSION_CUBEMAP,
    SKY_VERSION_CUBEMAP_HALF_RES,
    SKY_VERSION_CUBEMAP_QUARTER_RES,

    SKY_VERSION_BACKGROUND_MULTIVIEW,
    SKY_VERSION_HALF_RES_MULTIVIEW,
    SKY_VERSION_QUARTER_RES_MULTIVIEW,

    SKY_VERSION_MAX
  };

  struct SkyShaderData : public RendererRD::MaterialStorage::ShaderData {
    bool valid = false;
    RID version;

    PipelineCacheRD pipelines[SKY_VERSION_MAX];
    Vector<shader::ShaderCompiler::GeneratedCode::Texture> texture_uniforms;

    Vector<uint32_t> ubo_offsets;
    uint32_t ubo_size = 0;

    String code;

    bool uses_time = false;
    bool uses_position = false;
    bool uses_half_res = false;
    bool uses_quarter_res = false;
    bool uses_light = false;

    virtual void set_code(const String& p_Code);
    virtual bool is_animated() const;
    virtual bool casts_shadows() const;
    virtual RS::ShaderNativeSourceCode get_native_source_code() const;

    SkyShaderData() {}
    virtual ~SkyShaderData();
  };
	/* Sky shader */

	struct SkyShader {
		SkyShaderRD shader;
		shader::ShaderCompiler compiler;

		RID default_shader;
		RID default_material;
		RID default_shader_rd;
	} sky_shader;


  void _render_sky(RD::DrawListID p_list, float p_time, RID p_fb, PipelineCacheRD* p_pipeline, RID p_uniform_set, RID p_texture_set, const Projection& p_projection,
                   const Basis& p_orientation, const Vector3& p_position, float p_luminance_multiplier);

 public:
  struct SkyPushConstant {
    float orientation[12];       // 48 - 48
    float projection[4];         // 16 - 64
    float position[3];           // 12 - 76
    float time;                  // 4 - 80
    float pad[3];                // 12 - 92
    float luminance_multiplier;  // 4 - 96
                                 // 128 is the max size of a push constant. We can replace "pad" but we can't add any more.
  };

  struct SkySceneState {
    struct UBO {
      float combined_reprojection[RendererSceneRender::MAX_RENDER_VIEWS][16];  // 2 x 64 - 128
      float view_inv_projections[RendererSceneRender::MAX_RENDER_VIEWS][16];   // 2 x 64 - 256
      float view_eye_offsets[RendererSceneRender::MAX_RENDER_VIEWS][4];        // 2 x 16 - 288

      uint32_t volumetric_fog_enabled;     // 4 - 292
      float volumetric_fog_inv_length;     // 4 - 296
      float volumetric_fog_detail_spread;  // 4 - 300
      float volumetric_fog_sky_affect;     // 4 - 304

      uint32_t fog_enabled;   // 4 - 308
      float fog_sky_affect;   // 4 - 312
      float fog_density;      // 4 - 316
      float fog_sun_scatter;  // 4 - 320

      float fog_light_color[3];      // 12 - 332
      float fog_aerial_perspective;  // 4 - 336

      float z_far;                       // 4 - 340
      uint32_t directional_light_count;  // 4 - 344
      uint32_t pad1;                     // 4 - 348
      uint32_t pad2;                     // 4 - 352
    };

    UBO ubo;

    uint32_t view_count = 1;
    Transform3D cam_transform;
    Projection cam_projection;

    SkyDirectionalLightData* directional_lights = nullptr;
    SkyDirectionalLightData* last_frame_directional_lights = nullptr;
    uint32_t max_directional_lights;
    uint32_t last_frame_directional_light_count;
    RID directional_light_buffer;
    RID uniform_set;
    RID uniform_buffer;
    RID fog_uniform_set;
    RID default_fog_uniform_set;

    RID fog_shader;
    RID fog_material;
    RID fog_only_texture_uniform_set;
  } sky_scene_state;
  const int SAMPLERS_BINDING_FIRST_INDEX = 4;

};
}  // namespace lain::RendererRD

#endif