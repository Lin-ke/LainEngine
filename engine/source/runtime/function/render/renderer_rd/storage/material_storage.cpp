#include "material_storage.h"
using namespace lain::RendererRD;
using namespace lain;
MaterialStorage* MaterialStorage::p_singleton = nullptr;

lain::RendererRD::MaterialStorage::MaterialStorage() {
  p_singleton = this;
}

lain::RendererRD::MaterialStorage::~MaterialStorage() {
  p_singleton = nullptr;
}

void MaterialStorage::shader_initialize(RID p_rid) {
  Shader shader;
  shader.data = nullptr;
  shader.type = SHADER_TYPE_MAX;
  shader_owner.initialize_rid(p_rid, shader);
}

void lain::RendererRD::MaterialStorage::ShaderData::set_path_hint(const String& p_hint) {
  path = p_hint;
}

// 这里的方法都是对shader做反射

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



void Mate
