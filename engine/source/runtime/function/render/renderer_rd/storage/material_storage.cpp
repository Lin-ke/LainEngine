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

void MaterialStorage::global_shader_parameter_add(const StringName& p_name, RS::GlobalShaderParameterType p_type, const Variant& p_value) {}

void MaterialStorage::shader_initialize(RID p_rid) {
    Shader shader;
    shader.data = nullptr;
    shader.type = SHADER_TYPE_MAX;
    shader_owner.initialize_rid(p_rid, shader);
}
