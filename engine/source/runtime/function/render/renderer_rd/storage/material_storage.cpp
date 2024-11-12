#include "material_storage.h"
#include "core/variant/variant_converters.h"
using namespace lain::RendererRD;
using namespace lain;
MaterialStorage* MaterialStorage::p_singleton = nullptr;

lain::RendererRD::MaterialStorage::MaterialStorage() {
  p_singleton = this;
}

lain::RendererRD::MaterialStorage::~MaterialStorage() {
  p_singleton = nullptr;
}

///////////////////////////////////////////////////////////////////////////
// UBI helper functions

static void _fill_std140_variant_ubo_value(shader::ShaderLanguage::DataType type, int p_array_size, const Variant& value, uint8_t* data, bool p_linear_color) {
  switch (type) {
    case shader::ShaderLanguage::TYPE_BOOL: {
      uint32_t* gui = (uint32_t*)data;

      if (p_array_size > 0) {
        PackedInt32Array ba = value;
        for (int i = 0; i < ba.size(); i++) {
          ba.set(i, ba[i] ? 1 : 0);
        }
        write_array_std140<int32_t>(ba, gui, p_array_size, 4);
      } else {
        bool v = value;
        gui[0] = v ? 1 : 0;
      }
    } break;
    case shader::ShaderLanguage::TYPE_BVEC2: {
      uint32_t* gui = (uint32_t*)data;

      if (p_array_size > 0) {
        PackedInt32Array ba = convert_array_std140<Vector2i, int32_t>(value);
        for (int i = 0; i < ba.size(); i++) {
          ba.set(i, ba[i] ? 1 : 0);
        }
        write_array_std140<Vector2i>(ba, gui, p_array_size, 4);
      } else {
        uint32_t v = value;
        gui[0] = v & 1 ? 1 : 0;
        gui[1] = v & 2 ? 1 : 0;
      }
    } break;
    case shader::ShaderLanguage::TYPE_BVEC3: {
      uint32_t* gui = (uint32_t*)data;

      if (p_array_size > 0) {
        PackedInt32Array ba = convert_array_std140<Vector3i, int32_t>(value);
        for (int i = 0; i < ba.size(); i++) {
          ba.set(i, ba[i] ? 1 : 0);
        }
        write_array_std140<Vector3i>(ba, gui, p_array_size, 4);
      } else {
        uint32_t v = value;
        gui[0] = (v & 1) ? 1 : 0;
        gui[1] = (v & 2) ? 1 : 0;
        gui[2] = (v & 4) ? 1 : 0;
      }
    } break;
    case shader::ShaderLanguage::TYPE_BVEC4: {
      uint32_t* gui = (uint32_t*)data;

      if (p_array_size > 0) {
        PackedInt32Array ba = convert_array_std140<Vector4i, int32_t>(value);
        for (int i = 0; i < ba.size(); i++) {
          ba.set(i, ba[i] ? 1 : 0);
        }
        write_array_std140<Vector4i>(ba, gui, p_array_size, 4);
      } else {
        uint32_t v = value;
        gui[0] = (v & 1) ? 1 : 0;
        gui[1] = (v & 2) ? 1 : 0;
        gui[2] = (v & 4) ? 1 : 0;
        gui[3] = (v & 8) ? 1 : 0;
      }
    } break;
    case shader::ShaderLanguage::TYPE_INT: {
      int32_t* gui = (int32_t*)data;

      if (p_array_size > 0) {
        const PackedInt32Array& iv = value;
        write_array_std140<int32_t>(iv, gui, p_array_size, 4);
      } else {
        int v = value;
        gui[0] = v;
      }
    } break;
    case shader::ShaderLanguage::TYPE_IVEC2: {
      int32_t* gui = (int32_t*)data;

      if (p_array_size > 0) {
        const PackedInt32Array& iv = convert_array_std140<Vector2i, int32_t>(value);
        write_array_std140<Vector2i>(iv, gui, p_array_size, 4);
      } else {
        Vector2i v = convert_to_vector<Vector2i>(value);
        gui[0] = v.x;
        gui[1] = v.y;
      }
    } break;
    case shader::ShaderLanguage::TYPE_IVEC3: {
      int32_t* gui = (int32_t*)data;

      if (p_array_size > 0) {
        const PackedInt32Array& iv = convert_array_std140<Vector3i, int32_t>(value);
        write_array_std140<Vector3i>(iv, gui, p_array_size, 4);
      } else {
        Vector3i v = convert_to_vector<Vector3i>(value);
        gui[0] = v.x;
        gui[1] = v.y;
        gui[2] = v.z;
      }
    } break;
    case shader::ShaderLanguage::TYPE_IVEC4: {
      int32_t* gui = (int32_t*)data;

      if (p_array_size > 0) {
        const PackedInt32Array& iv = convert_array_std140<Vector4i, int32_t>(value);
        write_array_std140<Vector4i>(iv, gui, p_array_size, 4);
      } else {
        Vector4i v = convert_to_vector<Vector4i>(value);
        gui[0] = v.x;
        gui[1] = v.y;
        gui[2] = v.z;
        gui[3] = v.w;
      }
    } break;
    case shader::ShaderLanguage::TYPE_UINT: {
      uint32_t* gui = (uint32_t*)data;

      if (p_array_size > 0) {
        const PackedInt32Array& iv = value;
        write_array_std140<uint32_t>(iv, gui, p_array_size, 4);
      } else {
        int v = value;
        gui[0] = v;
      }
    } break;
    case shader::ShaderLanguage::TYPE_UVEC2: {
      uint32_t* gui = (uint32_t*)data;

      if (p_array_size > 0) {
        const PackedInt32Array& iv = convert_array_std140<Vector2i, int32_t>(value);
        write_array_std140<Vector2i>(iv, gui, p_array_size, 4);
      } else {
        Vector2i v = convert_to_vector<Vector2i>(value);
        gui[0] = v.x;
        gui[1] = v.y;
      }
    } break;
    case shader::ShaderLanguage::TYPE_UVEC3: {
      uint32_t* gui = (uint32_t*)data;

      if (p_array_size > 0) {
        const PackedInt32Array& iv = convert_array_std140<Vector3i, int32_t>(value);
        write_array_std140<Vector3i>(iv, gui, p_array_size, 4);
      } else {
        Vector3i v = convert_to_vector<Vector3i>(value);
        gui[0] = v.x;
        gui[1] = v.y;
        gui[2] = v.z;
      }
    } break;
    case shader::ShaderLanguage::TYPE_UVEC4: {
      uint32_t* gui = (uint32_t*)data;

      if (p_array_size > 0) {
        const PackedInt32Array& iv = convert_array_std140<Vector4i, int32_t>(value);
        write_array_std140<Vector4i>(iv, gui, p_array_size, 4);
      } else {
        Vector4i v = convert_to_vector<Vector4i>(value);
        gui[0] = v.x;
        gui[1] = v.y;
        gui[2] = v.z;
        gui[3] = v.w;
      }
    } break;
    case shader::ShaderLanguage::TYPE_FLOAT: {
      float* gui = reinterpret_cast<float*>(data);

      if (p_array_size > 0) {
        const PackedFloat32Array& a = value;
        write_array_std140<float>(a, gui, p_array_size, 4);
      } else {
        float v = value;
        gui[0] = v;
      }
    } break;
    case shader::ShaderLanguage::TYPE_VEC2: {
      float* gui = reinterpret_cast<float*>(data);

      if (p_array_size > 0) {
        const PackedFloat32Array& a = convert_array_std140<Vector2, float>(value);
        write_array_std140<Vector2>(a, gui, p_array_size, 4);
      } else {
        Vector2 v = convert_to_vector<Vector2>(value);
        gui[0] = v.x;
        gui[1] = v.y;
      }
    } break;
    case shader::ShaderLanguage::TYPE_VEC3: {
      float* gui = reinterpret_cast<float*>(data);

      if (p_array_size > 0) {
        const PackedFloat32Array& a = convert_array_std140<Vector3, float>(value, p_linear_color);
        write_array_std140<Vector3>(a, gui, p_array_size, 4);
      } else {
        Vector3 v = convert_to_vector<Vector3>(value, p_linear_color);
        gui[0] = v.x;
        gui[1] = v.y;
        gui[2] = v.z;
      }
    } break;
    case shader::ShaderLanguage::TYPE_VEC4: {
      float* gui = reinterpret_cast<float*>(data);

      if (p_array_size > 0) {
        const PackedFloat32Array& a = convert_array_std140<Vector4, float>(value, p_linear_color);
        write_array_std140<Vector4>(a, gui, p_array_size, 4);
      } else {
        Vector4 v = convert_to_vector<Vector4>(value, p_linear_color);
        gui[0] = v.x;
        gui[1] = v.y;
        gui[2] = v.z;
        gui[3] = v.w;
      }
    } break;
    case shader::ShaderLanguage::TYPE_MAT2: {
      float* gui = reinterpret_cast<float*>(data);

      if (p_array_size > 0) {
        const PackedFloat32Array& a = value;
        int s = a.size();

        for (int i = 0, j = 0; i < p_array_size * 4; i += 4, j += 8) {
          if (i + 3 < s) {
            gui[j] = a[i];
            gui[j + 1] = a[i + 1];

            gui[j + 4] = a[i + 2];
            gui[j + 5] = a[i + 3];
          } else {
            gui[j] = 1;
            gui[j + 1] = 0;

            gui[j + 4] = 0;
            gui[j + 5] = 1;
          }
          gui[j + 2] = 0;  // ignored
          gui[j + 3] = 0;  // ignored
          gui[j + 6] = 0;  // ignored
          gui[j + 7] = 0;  // ignored
        }
      } else {
        Transform2D v = value;

        //in std140 members of mat2 are treated as vec4s
        gui[0] = v.columns[0][0];
        gui[1] = v.columns[0][1];
        gui[2] = 0;  // ignored
        gui[3] = 0;  // ignored

        gui[4] = v.columns[1][0];
        gui[5] = v.columns[1][1];
        gui[6] = 0;  // ignored
        gui[7] = 0;  // ignored
      }
    } break;
    case shader::ShaderLanguage::TYPE_MAT3: {
      float* gui = reinterpret_cast<float*>(data);

      if (p_array_size > 0) {
        const PackedFloat32Array& a = convert_array_std140<Basis, float>(value);
        const Basis default_basis;
        const int s = a.size();

        for (int i = 0, j = 0; i < p_array_size * 9; i += 9, j += 12) {
          if (i + 8 < s) {
            gui[j] = a[i];
            gui[j + 1] = a[i + 1];
            gui[j + 2] = a[i + 2];
            gui[j + 3] = 0;  // Ignored.

            gui[j + 4] = a[i + 3];
            gui[j + 5] = a[i + 4];
            gui[j + 6] = a[i + 5];
            gui[j + 7] = 0;  // Ignored.

            gui[j + 8] = a[i + 6];
            gui[j + 9] = a[i + 7];
            gui[j + 10] = a[i + 8];
            gui[j + 11] = 0;  // Ignored.
          } else {
            convert_item_std140(default_basis, gui + j);
          }
        }
      } else {
        convert_item_std140<Basis>(value, gui);
      }
    } break;
    case shader::ShaderLanguage::TYPE_MAT4: {
      float* gui = reinterpret_cast<float*>(data);

      if (p_array_size > 0) {
        const PackedFloat32Array& a = convert_array_std140<Projection, float>(value);
        write_array_std140<Projection>(a, gui, p_array_size, 16);
      } else {
        convert_item_std140<Projection>(value, gui);
      }
    } break;
    default: {
    }
  }
}

_FORCE_INLINE_ static void _fill_std140_ubo_value(shader::ShaderLanguage::DataType type, const Vector<shader::ShaderLanguage::ConstantNode::Value>& value, uint8_t* data) {
  switch (type) {
    case shader::ShaderLanguage::TYPE_BOOL: {
      uint32_t* gui = (uint32_t*)data;
      gui[0] = value[0].boolean ? 1 : 0;
    } break;
    case shader::ShaderLanguage::TYPE_BVEC2: {
      uint32_t* gui = (uint32_t*)data;
      gui[0] = value[0].boolean ? 1 : 0;
      gui[1] = value[1].boolean ? 1 : 0;

    } break;
    case shader::ShaderLanguage::TYPE_BVEC3: {
      uint32_t* gui = (uint32_t*)data;
      gui[0] = value[0].boolean ? 1 : 0;
      gui[1] = value[1].boolean ? 1 : 0;
      gui[2] = value[2].boolean ? 1 : 0;

    } break;
    case shader::ShaderLanguage::TYPE_BVEC4: {
      uint32_t* gui = (uint32_t*)data;
      gui[0] = value[0].boolean ? 1 : 0;
      gui[1] = value[1].boolean ? 1 : 0;
      gui[2] = value[2].boolean ? 1 : 0;
      gui[3] = value[3].boolean ? 1 : 0;

    } break;
    case shader::ShaderLanguage::TYPE_INT: {
      int32_t* gui = (int32_t*)data;
      gui[0] = value[0].sint;

    } break;
    case shader::ShaderLanguage::TYPE_IVEC2: {
      int32_t* gui = (int32_t*)data;

      for (int i = 0; i < 2; i++) {
        gui[i] = value[i].sint;
      }

    } break;
    case shader::ShaderLanguage::TYPE_IVEC3: {
      int32_t* gui = (int32_t*)data;

      for (int i = 0; i < 3; i++) {
        gui[i] = value[i].sint;
      }

    } break;
    case shader::ShaderLanguage::TYPE_IVEC4: {
      int32_t* gui = (int32_t*)data;

      for (int i = 0; i < 4; i++) {
        gui[i] = value[i].sint;
      }

    } break;
    case shader::ShaderLanguage::TYPE_UINT: {
      uint32_t* gui = (uint32_t*)data;
      gui[0] = value[0].uint;

    } break;
    case shader::ShaderLanguage::TYPE_UVEC2: {
      int32_t* gui = (int32_t*)data;

      for (int i = 0; i < 2; i++) {
        gui[i] = value[i].uint;
      }
    } break;
    case shader::ShaderLanguage::TYPE_UVEC3: {
      int32_t* gui = (int32_t*)data;

      for (int i = 0; i < 3; i++) {
        gui[i] = value[i].uint;
      }

    } break;
    case shader::ShaderLanguage::TYPE_UVEC4: {
      int32_t* gui = (int32_t*)data;

      for (int i = 0; i < 4; i++) {
        gui[i] = value[i].uint;
      }
    } break;
    case shader::ShaderLanguage::TYPE_FLOAT: {
      float* gui = reinterpret_cast<float*>(data);
      gui[0] = value[0].real;

    } break;
    case shader::ShaderLanguage::TYPE_VEC2: {
      float* gui = reinterpret_cast<float*>(data);

      for (int i = 0; i < 2; i++) {
        gui[i] = value[i].real;
      }

    } break;
    case shader::ShaderLanguage::TYPE_VEC3: {
      float* gui = reinterpret_cast<float*>(data);

      for (int i = 0; i < 3; i++) {
        gui[i] = value[i].real;
      }

    } break;
    case shader::ShaderLanguage::TYPE_VEC4: {
      float* gui = reinterpret_cast<float*>(data);

      for (int i = 0; i < 4; i++) {
        gui[i] = value[i].real;
      }
    } break;
    case shader::ShaderLanguage::TYPE_MAT2: {
      float* gui = reinterpret_cast<float*>(data);

      //in std140 members of mat2 are treated as vec4s
      gui[0] = value[0].real;
      gui[1] = value[1].real;
      gui[2] = 0;
      gui[3] = 0;
      gui[4] = value[2].real;
      gui[5] = value[3].real;
      gui[6] = 0;
      gui[7] = 0;
    } break;
    case shader::ShaderLanguage::TYPE_MAT3: {
      float* gui = reinterpret_cast<float*>(data);

      gui[0] = value[0].real;
      gui[1] = value[1].real;
      gui[2] = value[2].real;
      gui[3] = 0;
      gui[4] = value[3].real;
      gui[5] = value[4].real;
      gui[6] = value[5].real;
      gui[7] = 0;
      gui[8] = value[6].real;
      gui[9] = value[7].real;
      gui[10] = value[8].real;
      gui[11] = 0;
    } break;
    case shader::ShaderLanguage::TYPE_MAT4: {
      float* gui = reinterpret_cast<float*>(data);

      for (int i = 0; i < 16; i++) {
        gui[i] = value[i].real;
      }
    } break;
    default: {
    }
  }
}

_FORCE_INLINE_ static void _fill_std140_ubo_empty(shader::ShaderLanguage::DataType type, int p_array_size, uint8_t* data) {
  if (p_array_size <= 0) {
    p_array_size = 1;
  }

  switch (type) {
    case shader::ShaderLanguage::TYPE_BOOL:
    case shader::ShaderLanguage::TYPE_INT:
    case shader::ShaderLanguage::TYPE_UINT:
    case shader::ShaderLanguage::TYPE_FLOAT: {
      memset(data, 0, 4 * p_array_size);
    } break;
    case shader::ShaderLanguage::TYPE_BVEC2:
    case shader::ShaderLanguage::TYPE_IVEC2:
    case shader::ShaderLanguage::TYPE_UVEC2:
    case shader::ShaderLanguage::TYPE_VEC2: {
      memset(data, 0, 8 * p_array_size);
    } break;
    case shader::ShaderLanguage::TYPE_BVEC3:
    case shader::ShaderLanguage::TYPE_IVEC3:
    case shader::ShaderLanguage::TYPE_UVEC3:
    case shader::ShaderLanguage::TYPE_VEC3: {
      memset(data, 0, 12 * p_array_size);
    } break;
    case shader::ShaderLanguage::TYPE_BVEC4:
    case shader::ShaderLanguage::TYPE_IVEC4:
    case shader::ShaderLanguage::TYPE_UVEC4:
    case shader::ShaderLanguage::TYPE_VEC4: {
      memset(data, 0, 16 * p_array_size);
    } break;
    case shader::ShaderLanguage::TYPE_MAT2: {
      memset(data, 0, 32 * p_array_size);
    } break;
    case shader::ShaderLanguage::TYPE_MAT3: {
      memset(data, 0, 48 * p_array_size);
    } break;
    case shader::ShaderLanguage::TYPE_MAT4: {
      memset(data, 0, 64 * p_array_size);
    } break;

    default: {
    }
  }
}

void MaterialStorage::material_set_data_request_function(ShaderType p_shader_type, MaterialStorage::MaterialDataRequestFunction p_function) {
  ERR_FAIL_INDEX(p_shader_type, SHADER_TYPE_MAX);
  material_data_request_func[p_shader_type] = p_function;
}

MaterialStorage::MaterialDataRequestFunction MaterialStorage::material_get_data_request_function(ShaderType p_shader_type) {
  ERR_FAIL_INDEX_V(p_shader_type, SHADER_TYPE_MAX, nullptr);
  return material_data_request_func[p_shader_type];
}

MaterialStorage::Samplers MaterialStorage::samplers_rd_allocate(float p_mipmap_bias) const {
  Samplers samplers;
  samplers.mipmap_bias = p_mipmap_bias;
  samplers.use_nearest_mipmap_filter = GLOBAL_GET("rendering/textures/default_filters/use_nearest_mipmap_filter");
  samplers.anisotropic_filtering_level = int(GLOBAL_GET("rendering/textures/default_filters/anisotropic_filtering_level"));

  RD::SamplerFilter mip_filter = samplers.use_nearest_mipmap_filter ? RD::SAMPLER_FILTER_NEAREST : RD::SAMPLER_FILTER_LINEAR;
  float anisotropy_max = float(1 << samplers.anisotropic_filtering_level);

  for (int i = 1; i < RS::CANVAS_ITEM_TEXTURE_FILTER_MAX; i++) {
    for (int j = 1; j < RS::CANVAS_ITEM_TEXTURE_REPEAT_MAX; j++) {
      RD::SamplerState sampler_state;
      switch (i) {
        case RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST: {
          sampler_state.mag_filter = RD::SAMPLER_FILTER_NEAREST;
          sampler_state.min_filter = RD::SAMPLER_FILTER_NEAREST;
          sampler_state.max_lod = 0;
        } break;
        case RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR: {
          sampler_state.mag_filter = RD::SAMPLER_FILTER_LINEAR;
          sampler_state.min_filter = RD::SAMPLER_FILTER_LINEAR;
          sampler_state.max_lod = 0;
        } break;
        case RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS: {
          sampler_state.mag_filter = RD::SAMPLER_FILTER_NEAREST;
          sampler_state.min_filter = RD::SAMPLER_FILTER_NEAREST;
          sampler_state.mip_filter = mip_filter;
          sampler_state.lod_bias = samplers.mipmap_bias;
        } break;
        case RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS: {
          sampler_state.mag_filter = RD::SAMPLER_FILTER_LINEAR;
          sampler_state.min_filter = RD::SAMPLER_FILTER_LINEAR;
          sampler_state.mip_filter = mip_filter;
          sampler_state.lod_bias = samplers.mipmap_bias;

        } break;
        case RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS_ANISOTROPIC: {
          sampler_state.mag_filter = RD::SAMPLER_FILTER_NEAREST;
          sampler_state.min_filter = RD::SAMPLER_FILTER_NEAREST;
          sampler_state.mip_filter = mip_filter;
          sampler_state.lod_bias = samplers.mipmap_bias;
          sampler_state.use_anisotropy = true;
          sampler_state.anisotropy_max = anisotropy_max;
        } break;
        case RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS_ANISOTROPIC: {
          sampler_state.mag_filter = RD::SAMPLER_FILTER_LINEAR;
          sampler_state.min_filter = RD::SAMPLER_FILTER_LINEAR;
          sampler_state.mip_filter = mip_filter;
          sampler_state.lod_bias = samplers.mipmap_bias;
          sampler_state.use_anisotropy = true;
          sampler_state.anisotropy_max = anisotropy_max;

        } break;
        default: {
        }
      }
      switch (j) {
        case RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED: {
          sampler_state.repeat_u = RD::SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE;
          sampler_state.repeat_v = RD::SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE;
          sampler_state.repeat_w = RD::SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE;

        } break;
        case RS::CANVAS_ITEM_TEXTURE_REPEAT_ENABLED: {
          sampler_state.repeat_u = RD::SAMPLER_REPEAT_MODE_REPEAT;
          sampler_state.repeat_v = RD::SAMPLER_REPEAT_MODE_REPEAT;
          sampler_state.repeat_w = RD::SAMPLER_REPEAT_MODE_REPEAT;
        } break;
        case RS::CANVAS_ITEM_TEXTURE_REPEAT_MIRROR: {
          sampler_state.repeat_u = RD::SAMPLER_REPEAT_MODE_MIRRORED_REPEAT;
          sampler_state.repeat_v = RD::SAMPLER_REPEAT_MODE_MIRRORED_REPEAT;
          sampler_state.repeat_w = RD::SAMPLER_REPEAT_MODE_MIRRORED_REPEAT;
        } break;
        default: {
        }
      }

      samplers.rids[i][j] = RD::get_singleton()->sampler_create(sampler_state);
    }
  }

  return samplers;
}

void MaterialStorage::samplers_rd_free(Samplers &p_samplers) const {
	for (int i = 1; i < RS::CANVAS_ITEM_TEXTURE_FILTER_MAX; i++) {
		for (int j = 1; j < RS::CANVAS_ITEM_TEXTURE_REPEAT_MAX; j++) {
			if (p_samplers.rids[i][j].is_valid()) {
				RD::get_singleton()->free(p_samplers.rids[i][j]);
				p_samplers.rids[i][j] = RID();
			}
		}
	}
}

int32_t lain::RendererRD::MaterialStorage::_global_shader_uniform_allocate(uint32_t p_elements) {
  int32_t idx = 0;
  while (idx + p_elements <= global_shader_uniforms.buffer_size) {
    if (global_shader_uniforms.buffer_usage[idx].elements == 0) {
      bool valid = true;
      for (uint32_t i = 1; i < p_elements; i++) {
        if (global_shader_uniforms.buffer_usage[idx + i].elements > 0) {
          valid = false;
          idx += i + global_shader_uniforms.buffer_usage[idx + i].elements;
          break;
        }
      }

      if (!valid) {
        continue;  //if not valid, idx is in new position
      }

      return idx;
    } else {
      idx += global_shader_uniforms.buffer_usage[idx].elements;
    }
  }

  return -1;
}

int32_t lain::RendererRD::MaterialStorage::global_shader_parameters_instance_allocate(RID p_instance) {
  ERR_FAIL_COND_V(global_shader_uniforms.instance_buffer_pos.has(p_instance), -1);
  int32_t pos = _global_shader_uniform_allocate(shader::ShaderLanguage::MAX_INSTANCE_UNIFORM_INDICES);  // 分配MAX_INSTANCE_UNIFORM_INDICES个
  global_shader_uniforms.instance_buffer_pos[p_instance] = pos;                                         //save anyway
  ERR_FAIL_COND_V_MSG(pos < 0, -1, "Too many instances using shader instance variables. Increase buffer size in Project Settings.");
  global_shader_uniforms.buffer_usage[pos].elements = shader::ShaderLanguage::MAX_INSTANCE_UNIFORM_INDICES;
  return pos;
}


void MaterialStorage::global_shader_parameters_instance_free(RID p_instance) {
  ERR_FAIL_COND(!global_shader_uniforms.instance_buffer_pos.has(p_instance));
  int32_t pos = global_shader_uniforms.instance_buffer_pos[p_instance];
  if (pos >= 0) {
    global_shader_uniforms.buffer_usage[pos].elements = 0;
  }
  global_shader_uniforms.instance_buffer_pos.erase(p_instance);
}

void MaterialStorage::global_shader_parameters_instance_update(RID p_instance, int p_index, const Variant& p_value, int p_flags_count) {
  if (!global_shader_uniforms.instance_buffer_pos.has(p_instance)) {
    return;  //just not allocated, ignore
  }
  int32_t pos = global_shader_uniforms.instance_buffer_pos[p_instance];

  if (pos < 0) {
    return;  //again, not allocated, ignore
  }
  ERR_FAIL_INDEX(p_index, shader::ShaderLanguage::MAX_INSTANCE_UNIFORM_INDICES);

  Variant::Type value_type = p_value.get_type();
  ERR_FAIL_COND_MSG(p_value.get_type() > Variant::COLOR,
                    "Unsupported variant type for instance parameter: " + Variant::get_type_name(value_type));  //anything greater not supported

  const shader::ShaderLanguage::DataType datatype_from_value[Variant::COLOR + 1] = {
      shader::ShaderLanguage::TYPE_MAX,    //nil
      shader::ShaderLanguage::TYPE_BOOL,   //bool
      shader::ShaderLanguage::TYPE_INT,    //int
      shader::ShaderLanguage::TYPE_FLOAT,  //float
      shader::ShaderLanguage::TYPE_MAX,    //string
      shader::ShaderLanguage::TYPE_VEC2,   //vec2
      shader::ShaderLanguage::TYPE_IVEC2,  //vec2i
      shader::ShaderLanguage::TYPE_VEC4,   //rect2
      shader::ShaderLanguage::TYPE_IVEC4,  //rect2i
      shader::ShaderLanguage::TYPE_VEC3,   // vec3
      shader::ShaderLanguage::TYPE_IVEC3,  //vec3i
      shader::ShaderLanguage::TYPE_MAX,    //xform2d not supported here
      shader::ShaderLanguage::TYPE_VEC4,   //vec4
      shader::ShaderLanguage::TYPE_IVEC4,  //vec4i
      shader::ShaderLanguage::TYPE_VEC4,   //plane
      shader::ShaderLanguage::TYPE_VEC4,   //quat
      shader::ShaderLanguage::TYPE_MAX,    //aabb not supported here
      shader::ShaderLanguage::TYPE_MAX,    //basis not supported here
      shader::ShaderLanguage::TYPE_MAX,    //xform not supported here
      shader::ShaderLanguage::TYPE_MAX,    //projection not supported here
      shader::ShaderLanguage::TYPE_VEC4    //color
  };

  shader::ShaderLanguage::DataType datatype = shader::ShaderLanguage::TYPE_MAX;
  if (value_type == Variant::INT && p_flags_count > 0) {
    switch (p_flags_count) {
      case 1:
        datatype = shader::ShaderLanguage::TYPE_BVEC2;
        break;
      case 2:
        datatype = shader::ShaderLanguage::TYPE_BVEC3;
        break;
      case 3:
        datatype = shader::ShaderLanguage::TYPE_BVEC4;
        break;
    }
  } else {
    datatype = datatype_from_value[value_type];
  }
  ERR_FAIL_COND_MSG(datatype == shader::ShaderLanguage::TYPE_MAX,
                    "Unsupported variant type for instance parameter: " + Variant::get_type_name(value_type));  //anything greater not supported

  pos += p_index;

  // _fill_std140_variant_ubo_value(datatype, 0, p_value, (uint8_t *)&global_shader_uniforms.buffer_values[pos], true); //instances always use linear color in this renderer
  // _global_shader_uniform_mark_buffer_dirty(pos, 1);
}

void MaterialStorage::shader_initialize(RID p_rid) {
  Shader shader;
  shader.data = nullptr;
  shader.type = SHADER_TYPE_MAX;
  shader_owner.initialize_rid(p_rid, shader);
}

void MaterialStorage::shader_free(RID p_rid) {
  Shader* shader = shader_owner.get_or_null(p_rid);
  ERR_FAIL_NULL(shader);

  //make material unreference this
  while (shader->owners.size()) {
    material_set_shader((*shader->owners.begin())->self, RID());
  }

  //clear data if exists
  if (shader->data) {
    memdelete(shader->data);
  }
  shader_owner.free(p_rid);
}

void MaterialStorage::shader_set_path_hint(RID p_shader, const String& p_path) {
  Shader* shader = shader_owner.get_or_null(p_shader);
  ERR_FAIL_NULL(shader);

  shader->path_hint = p_path;
  if (shader->data) {
    shader->data->set_path_hint(p_path);
  }
}
String MaterialStorage::shader_get_code(RID p_shader) const {
  Shader* shader = shader_owner.get_or_null(p_shader);
  ERR_FAIL_NULL_V(shader, String());
  return shader->code;
}
void MaterialStorage::get_shader_parameter_list(RID p_shader, List<PropertyInfo>* p_param_list) const {
  Shader* shader = shader_owner.get_or_null(p_shader);
  ERR_FAIL_NULL(shader);
  if (shader->data) {
    return shader->data->get_shader_uniform_list(p_param_list);
  }
}

Variant MaterialStorage::shader_get_parameter_default(RID p_shader, const StringName &p_param) const {
	Shader *shader = shader_owner.get_or_null(p_shader);
	ERR_FAIL_NULL_V(shader, Variant());
	if (shader->data) {
		return shader->data->get_default_parameter(p_param);
	}
	return Variant();
}


void MaterialStorage::shader_set_default_texture_parameter(RID p_shader, const StringName& p_name, RID p_texture, int p_index) {
  Shader* shader = shader_owner.get_or_null(p_shader);
  ERR_FAIL_NULL(shader);

  if (p_texture.is_valid() && TextureStorage::get_singleton()->owns_texture(p_texture)) {
    if (!shader->default_texture_parameter.has(p_name)) {
      shader->default_texture_parameter[p_name] = HashMap<int, RID>();
    }
    shader->default_texture_parameter[p_name][p_index] = p_texture;
  } else {
    if (shader->default_texture_parameter.has(p_name) && shader->default_texture_parameter[p_name].has(p_index)) {
      shader->default_texture_parameter[p_name].erase(p_index);

      if (shader->default_texture_parameter[p_name].is_empty()) {
        shader->default_texture_parameter.erase(p_name);
      }
    }
  }
  if (shader->data) {
    shader->data->set_default_texture_parameter(p_name, p_texture, p_index);
  }
  for (Material* E : shader->owners) {
    Material* material = E;
    _material_queue_update(material, false, true);
  }
}

RID MaterialStorage::shader_get_default_texture_parameter(RID p_shader, const StringName& p_name, int p_index) const {
  Shader* shader = shader_owner.get_or_null(p_shader);
  ERR_FAIL_NULL_V(shader, RID());
  if (shader->default_texture_parameter.has(p_name) && shader->default_texture_parameter[p_name].has(p_index)) {
    return shader->default_texture_parameter[p_name][p_index];
  }

  return RID();
}

RS::ShaderNativeSourceCode MaterialStorage::shader_get_native_source_code(RID p_shader) const {
  Shader* shader = shader_owner.get_or_null(p_shader);
  ERR_FAIL_NULL_V(shader, RS::ShaderNativeSourceCode());
  if (shader->data) {
    return shader->data->get_native_source_code();
  }
  return RS::ShaderNativeSourceCode();
}

RID MaterialStorage::material_allocate() {
  return material_owner.allocate_rid();
}

void MaterialStorage::material_initialize(RID p_rid) {
  material_owner.initialize_rid(p_rid);
  Material* material = material_owner.get_or_null(p_rid);
  material->self = p_rid;
}

void MaterialStorage::material_free(RID p_rid) {
  Material* material = material_owner.get_or_null(p_rid);
  ERR_FAIL_NULL(material);

  // Need to clear texture arrays to prevent spin locking of their RID's.
  // This happens when the app is being closed.
  for (KeyValue<StringName, Variant>& E : material->params) {
    if (E.value.get_type() == Variant::ARRAY) {
      Array(E.value).clear();
    }
  }

  material_set_shader(p_rid, RID());  //clean up shader
  material->dependency.deleted_notify(p_rid);

  material_owner.free(p_rid);
}

void MaterialStorage::material_set_render_priority(RID p_material, int priority) {
  Material* material = material_owner.get_or_null(p_material);
  ERR_FAIL_NULL(material);
  material->priority = priority;
  if (material->data) {
    material->data->set_render_priority(priority);
  }
  material->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_MATERIAL);
}

void MaterialStorage::material_set_shader(RID p_material, RID p_shader) {
  Material* material = material_owner.get_or_null(p_material);
  ERR_FAIL_NULL(material);

  if (material->data) {
    memdelete(material->data);
    material->data = nullptr;
  }

  if (material->shader) {
    material->shader->owners.erase(material);
    material->shader = nullptr;
    material->shader_type = SHADER_TYPE_MAX;
  }

  if (p_shader.is_null()) {  // 删除shader
    material->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_MATERIAL);
    material->shader_id = 0;
    return;
  }

  Shader* shader = get_shader(p_shader);
  ERR_FAIL_NULL(shader);
  material->shader = shader;
  material->shader_type = shader->type;
  material->shader_id = p_shader.get_local_index();
  shader->owners.insert(material);

  if (shader->type == SHADER_TYPE_MAX) {
    return;
  }

  ERR_FAIL_NULL(shader->data);

  {
    material->data = material_data_request_func[shader->type](shader->data);
    material->data->self = p_material;
    material->data->set_next_pass(material->next_pass);
    material->data->set_render_priority(material->priority);
  }
  //updating happens later
  material->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_MATERIAL);
  _material_queue_update(material, true, true);
}
void MaterialStorage::material_set_param(RID p_material, const StringName& p_param, const Variant& p_value) {
  Material* material = material_owner.get_or_null(p_material);
  ERR_FAIL_NULL(material);

  if (p_value.get_type() == Variant::NIL) {
    material->params.erase(p_param);
  } else {
    ERR_FAIL_COND(p_value.get_type() == Variant::OBJECT);  //object not allowed
    material->params[p_param] = p_value;
  }

  if (material->shader && material->shader->data) {  //shader is valid
    bool is_texture = material->shader->data->is_parameter_texture(p_param);
    _material_queue_update(material, !is_texture, is_texture);
  } else {
    _material_queue_update(material, true, true);
  }
}
Variant MaterialStorage::material_get_param(RID p_material, const StringName& p_param) const {
  Material* material = material_owner.get_or_null(p_material);
  ERR_FAIL_NULL_V(material, Variant());
  if (material->params.has(p_param)) {
    return material->params[p_param];
  } else {
    return Variant();
  }
}

bool MaterialStorage::free(RID p_rid) {
	if (owns_shader(p_rid)) {
		shader_free(p_rid);
		return true;
	} else if (owns_material(p_rid)) {
		material_free(p_rid);
		return true;
	}

	return false;
}

void lain::RendererRD::MaterialStorage::shader_set_code(RID p_shader, const String& p_code) {
  Shader* shader = shader_owner.get_or_null(p_shader);
  ERR_FAIL_NULL(shader);

  shader->code = p_code;
  String mode_string = shader::ShaderLanguage::get_shader_type(p_code);

  ShaderType new_type;
  if (mode_string == "canvas_item") {
    new_type = SHADER_TYPE_2D;
  } else if (mode_string == "particles") {
    new_type = SHADER_TYPE_PARTICLES;
  } else if (mode_string == "spatial") {
    new_type = SHADER_TYPE_3D;
  } else if (mode_string == "sky") {
    new_type = SHADER_TYPE_SKY;
  } else if (mode_string == "fog") {
    new_type = SHADER_TYPE_FOG;
  } else {
    new_type = SHADER_TYPE_MAX;
  }

  if (new_type != shader->type) {
    if (shader->data) {
      memdelete(shader->data);
      shader->data = nullptr;
    }

    for (Material* E : shader->owners) {
      Material* material = E;
      material->shader_type = new_type;
      if (material->data) {
        memdelete(material->data);
        material->data = nullptr;
      }
    }

    shader->type = new_type;

    if (new_type < SHADER_TYPE_MAX && shader_data_request_func[new_type]) {
      shader->data = shader_data_request_func[new_type]();
    } else {
      shader->type = SHADER_TYPE_MAX;  //invalid
    }

    for (Material* E : shader->owners) {
      Material* material = E;
      if (shader->data) {
        material->data = material_get_data_request_function(new_type)(shader->data);
        material->data->self = material->self;
        material->data->set_next_pass(material->next_pass);
        material->data->set_render_priority(material->priority);
      }
      material->shader_type = new_type;
    }
    // shader->data 说明shader有效， 重新设置texture paramter
    if (shader->data) {
      for (const KeyValue<StringName, HashMap<int, RID>>& E : shader->default_texture_parameter) {
        for (const KeyValue<int, RID>& E2 : E.value) {
          shader->data->set_default_texture_parameter(E.key, E2.value, E2.key);
        }
      }
    }
  }

  if (shader->data) {
    shader->data->set_path_hint(shader->path_hint);
    shader->data->set_code(p_code);
  }

  for (Material* E : shader->owners) {
    Material* material = E;
    material->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_MATERIAL);
    _material_queue_update(material, true, true);
  }
}

void lain::RendererRD::MaterialStorage::ShaderData::set_path_hint(const String& p_hint) {
  path = p_hint;
}

// uniform在 setcode里就填充了，方法是对shader进行compile

void lain::RendererRD::MaterialStorage::ShaderData::set_default_texture_parameter(const StringName& p_name, RID p_texture, int p_index) {
  if (!p_texture.is_valid()) {
    if (default_texture_params.has(p_name) && default_texture_params[p_name].has(p_index)) {
      default_texture_params[p_name].erase(p_index);

      if (default_texture_params[p_name].is_empty()) {
        default_texture_params.erase(p_name);
      }
    }
  } else {
    if (!default_texture_params.has(p_name)) {
      default_texture_params[p_name] = HashMap<int, RID>();
    }
    default_texture_params[p_name][p_index] = p_texture;
  }
}
Variant MaterialStorage::ShaderData::get_default_parameter(const StringName& p_parameter) const {
  if (uniforms.has(p_parameter)) {
    shader::ShaderLanguage::ShaderNode::Uniform uniform = uniforms[p_parameter];
    Vector<shader::ShaderLanguage::ConstantNode::Value> default_value = uniform.default_value;
    return shader::ShaderLanguage::constant_value_to_variant(default_value, uniform.type, uniform.array_size, uniform.hint);
  }
  return Variant();
}

void MaterialStorage::ShaderData::get_shader_uniform_list(List<PropertyInfo>* p_param_list) const {
  SortArray<Pair<StringName, int>, shader::ShaderLanguage::UniformOrderComparator> sorter;
  LocalVector<Pair<StringName, int>> filtered_uniforms;

  for (const KeyValue<StringName, shader::ShaderLanguage::ShaderNode::Uniform>& E : uniforms) {
    if (E.value.scope != shader::ShaderLanguage::ShaderNode::Uniform::SCOPE_LOCAL) {
      continue;
    }
    if (E.value.texture_order >= 0) {
      filtered_uniforms.push_back(Pair<StringName, int>(E.key, E.value.texture_order + 100000));
    } else {
      filtered_uniforms.push_back(Pair<StringName, int>(E.key, E.value.order));
    }
  }
  int uniform_count = filtered_uniforms.size();
  sorter.sort(filtered_uniforms.ptr(), uniform_count);

  String last_group;
  for (int i = 0; i < uniform_count; i++) {
    const StringName& uniform_name = filtered_uniforms[i].first;
    const shader::ShaderLanguage::ShaderNode::Uniform& uniform = uniforms[uniform_name];

    String group = uniform.group;
    if (!uniform.subgroup.is_empty()) {
      group += "::" + uniform.subgroup;
    }

    if (group != last_group) {
      PropertyInfo pi;
      pi.usage = PROPERTY_USAGE_GROUP;
      pi.name = group;
      p_param_list->push_back(pi);

      last_group = group;
    }

    PropertyInfo pi = shader::ShaderLanguage::uniform_to_property_info(uniform);
    pi.name = uniform_name;
    p_param_list->push_back(pi);
  }
}
void MaterialStorage::material_update_dependency(RID p_material, DependencyTracker* p_instance) {
  Material* material = material_owner.get_or_null(p_material);
  ERR_FAIL_NULL(material);
  p_instance->update_dependency(&material->dependency);
  if (material->next_pass.is_valid()) {
    material_update_dependency(material->next_pass, p_instance);
  }
}

void MaterialStorage::ShaderData::get_instance_param_list(List<RendererMaterialStorage::InstanceShaderParam>* p_param_list) const {
  for (const KeyValue<StringName, shader::ShaderLanguage::ShaderNode::Uniform>& E : uniforms) {
    if (E.value.scope != shader::ShaderLanguage::ShaderNode::Uniform::SCOPE_INSTANCE) {
      continue;
    }

    RendererMaterialStorage::InstanceShaderParam p;
    p.info = shader::ShaderLanguage::uniform_to_property_info(E.value);
    p.info.name = E.key;  //supply name
    p.index = E.value.instance_index;
    p.default_value = shader::ShaderLanguage::constant_value_to_variant(E.value.default_value, E.value.type, E.value.array_size, E.value.hint);
    p_param_list->push_back(p);
  }
}

bool MaterialStorage::ShaderData::is_parameter_texture(const StringName& p_param) const {
  if (!uniforms.has(p_param)) {
    return false;
  }

  return uniforms[p_param].texture_order >= 0;
}
void MaterialStorage::material_set_next_pass(RID p_material, RID p_next_material) {
  Material* material = material_owner.get_or_null(p_material);
  ERR_FAIL_NULL(material);

  if (material->next_pass == p_next_material) {
    return;
  }

  material->next_pass = p_next_material;
  if (material->data) {
    material->data->set_next_pass(p_next_material);
  }

  material->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_MATERIAL);
}
bool MaterialStorage::material_is_animated(RID p_material) {
  Material* material = material_owner.get_or_null(p_material);
  ERR_FAIL_NULL_V(material, false);
  if (material->shader && material->shader->data) {
    if (material->shader->data->is_animated()) {
      return true;
    } else if (material->next_pass.is_valid()) {
      return material_is_animated(material->next_pass);
    }
  }
  return false;  //by default nothing is animated
}

bool MaterialStorage::material_casts_shadows(RID p_material) {
  Material* material = material_owner.get_or_null(p_material);
  ERR_FAIL_NULL_V(material, true);
  if (material->shader && material->shader->data) {
    if (material->shader->data->casts_shadows()) {
      return true;
    } else if (material->next_pass.is_valid()) {
      return material_casts_shadows(material->next_pass);
    }
  }
  return true;  //by default everything casts shadows
}
void MaterialStorage::material_get_instance_shader_parameters(RID p_material, List<InstanceShaderParam>* r_parameters) {
  Material* material = material_owner.get_or_null(p_material);
  ERR_FAIL_NULL(material);
  if (material->shader && material->shader->data) {
    material->shader->data->get_instance_param_list(r_parameters);

    if (material->next_pass.is_valid()) {
      material_get_instance_shader_parameters(material->next_pass, r_parameters);
    }
  }
}

void MaterialStorage::shader_set_data_request_function(ShaderType p_shader_type, ShaderDataRequestFunction p_function) {
	ERR_FAIL_INDEX(p_shader_type, SHADER_TYPE_MAX);
	shader_data_request_func[p_shader_type] = p_function;
}


//加入队列
void MaterialStorage::_material_queue_update(Material* material, bool p_uniform, bool p_texture) {
  material->uniform_dirty = material->uniform_dirty || p_uniform;
  material->texture_dirty = material->texture_dirty || p_texture;

  if (material->update_element.in_list()) {
    return;
  }

  material_update_list.add(&material->update_element);
}

Vector<RD::Uniform> lain::RendererRD::MaterialStorage::Samplers::get_uniforms(int p_first_index) const {
  Vector<RD::Uniform> uniforms;

  // Binding ids are aligned with samplers_inc.glsl.
  uniforms.push_back(RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, p_first_index + 0, rids[RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST][RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED]));
  uniforms.push_back(RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, p_first_index + 1, rids[RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR][RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED]));
  uniforms.push_back(
      RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, p_first_index + 2, rids[RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS][RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED]));
  uniforms.push_back(
      RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, p_first_index + 3, rids[RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS][RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED]));
  uniforms.push_back(RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, p_first_index + 4,
                                 rids[RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS_ANISOTROPIC][RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED]));
  uniforms.push_back(
      RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, p_first_index + 5, rids[RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS_ANISOTROPIC][RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED]));
  uniforms.push_back(RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, p_first_index + 6, rids[RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST][RS::CANVAS_ITEM_TEXTURE_REPEAT_ENABLED]));
  uniforms.push_back(RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, p_first_index + 7, rids[RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR][RS::CANVAS_ITEM_TEXTURE_REPEAT_ENABLED]));
  uniforms.push_back(
      RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, p_first_index + 8, rids[RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS][RS::CANVAS_ITEM_TEXTURE_REPEAT_ENABLED]));
  uniforms.push_back(
      RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, p_first_index + 9, rids[RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS][RS::CANVAS_ITEM_TEXTURE_REPEAT_ENABLED]));
  uniforms.push_back(RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, p_first_index + 10,
                                 rids[RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS_ANISOTROPIC][RS::CANVAS_ITEM_TEXTURE_REPEAT_ENABLED]));
  uniforms.push_back(
      RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, p_first_index + 11, rids[RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS_ANISOTROPIC][RS::CANVAS_ITEM_TEXTURE_REPEAT_ENABLED]));

  return uniforms;
}

bool MaterialStorage::Samplers::is_valid() const {
  return rids[1][1].is_valid();
}

bool MaterialStorage::Samplers::is_null() const {
  return rids[1][1].is_null();
}

void lain::RendererRD::MaterialStorage::MaterialData::update_uniform_buffer(const HashMap<StringName, shader::ShaderLanguage::ShaderNode::Uniform>& p_uniforms,
                                                                            const uint32_t* p_uniform_offsets, const HashMap<StringName, Variant>& p_parameters,
                                                                            uint8_t* p_buffer, uint32_t p_buffer_size, bool p_use_linear_color) {
  MaterialStorage* material_storage = MaterialStorage::get_singleton();
  bool uses_global_buffer = false;

  for (const KeyValue<StringName, shader::ShaderLanguage::ShaderNode::Uniform>& E : p_uniforms) {
    if (E.value.order < 0) {
      continue;  // texture, does not go here
    }

    if (E.value.scope == shader::ShaderLanguage::ShaderNode::Uniform::SCOPE_INSTANCE) {
      continue;  //instance uniforms don't appear in the buffer
    }

    if (E.value.hint == shader::ShaderLanguage::ShaderNode::Uniform::HINT_SCREEN_TEXTURE ||
        E.value.hint == shader::ShaderLanguage::ShaderNode::Uniform::HINT_NORMAL_ROUGHNESS_TEXTURE ||
        E.value.hint == shader::ShaderLanguage::ShaderNode::Uniform::HINT_DEPTH_TEXTURE) {
      continue;
    }

    if (E.value.scope == shader::ShaderLanguage::ShaderNode::Uniform::SCOPE_GLOBAL) {
      //this is a global variable, get the index to it
      GlobalShaderUniforms::Variable* gv = material_storage->global_shader_uniforms.variables.getptr(E.key);
      uint32_t index = 0;
      if (gv) {
        index = gv->buffer_index;
      } else {
        WARN_PRINT("Shader uses global parameter '" + E.key + "', but it was removed at some point. Material will not display correctly.");
      }

      uint32_t offset = p_uniform_offsets[E.value.order];
      uint32_t* intptr = (uint32_t*)&p_buffer[offset];
      *intptr = index;
      uses_global_buffer = true;
      continue;
    }

    //regular uniform
    // 根据order取出offset，根据offset 从 p_buffer 中获得指针，传入 _fill_std140_variant_ubo_value 函数，其中提供了variant converts
    uint32_t offset = p_uniform_offsets[E.value.order];
#ifdef DEBUG_ENABLED
    uint32_t size = 0U;
    // The following code enforces a 16-byte alignment of uniform arrays.
    if (E.value.array_size > 0) {
      size = shader::ShaderLanguage::get_datatype_size(E.value.type) * E.value.array_size;
      int m = (16 * E.value.array_size);
      if ((size % m) != 0U) {
        size += m - (size % m);
      }
    } else {
      size = shader::ShaderLanguage::get_datatype_size(E.value.type);
    }
    ERR_CONTINUE(offset + size > p_buffer_size);
#endif
    uint8_t* data = &p_buffer[offset];
    HashMap<StringName, Variant>::ConstIterator V = p_parameters.find(E.key);

    if (V) {
      //user provided
      _fill_std140_variant_ubo_value(E.value.type, E.value.array_size, V->value, data, p_use_linear_color);

    } else if (E.value.default_value.size()) {
      //default value
      _fill_std140_ubo_value(E.value.type, E.value.default_value, data);
      //value=E.value.default_value;
    } else {
      //zero because it was not provided
      if ((E.value.type == shader::ShaderLanguage::TYPE_VEC3 || E.value.type == shader::ShaderLanguage::TYPE_VEC4) &&
          E.value.hint == shader::ShaderLanguage::ShaderNode::Uniform::HINT_SOURCE_COLOR) {
        //colors must be set as black, with alpha as 1.0
        _fill_std140_variant_ubo_value(E.value.type, E.value.array_size, Color(0, 0, 0, 1), data, p_use_linear_color);
      } else {
        //else just zero it out
        _fill_std140_ubo_empty(E.value.type, E.value.array_size, data);
      }
    }
  }

  if (uses_global_buffer != (global_buffer_E != nullptr)) {
    if (uses_global_buffer) {
      global_buffer_E = material_storage->global_shader_uniforms.materials_using_buffer.push_back(self);
    } else {
      material_storage->global_shader_uniforms.materials_using_buffer.erase(global_buffer_E);
      global_buffer_E = nullptr;
    }
  }
}
