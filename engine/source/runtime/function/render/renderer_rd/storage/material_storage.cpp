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
    // 如果data 存在就 Update default texture parameters，这个因果关系是为什么？
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
