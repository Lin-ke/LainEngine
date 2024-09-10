#ifndef MATERIAL_STORAGE_RDIMPL_H
#define MATERIAL_STORAGE_RDIMPL_H
//
#include "function/render/rendering_system/material_storage_api.h"
namespace lain::RendererRD {

class MaterialStorage : public RendererMaterialStorage {
public:
  enum ShaderType { SHADER_TYPE_2D, SHADER_TYPE_3D, SHADER_TYPE_PARTICLES, SHADER_TYPE_SKY, SHADER_TYPE_FOG, SHADER_TYPE_MAX };

 private:
  static MaterialStorage* p_singleton;
  MaterialStorage();
  ~MaterialStorage() override;
  // ShaderData
  struct ShaderData {
    String path;
    // HashMap<StringName, ShaderLanguage::ShaderNode::Uniform> uniforms;
    HashMap<StringName, HashMap<int, RID>> default_texture_params;

    virtual void set_path_hint(const String& p_hint);
    virtual void set_default_texture_parameter(const StringName& p_name, RID p_texture, int p_index);
    virtual Variant get_default_parameter(const StringName& p_parameter) const;
    virtual void get_shader_uniform_list(List<PropertyInfo>* p_param_list) const;
    virtual void get_instance_param_list(List<RendererMaterialStorage::InstanceShaderParam>* p_param_list) const;
    virtual bool is_parameter_texture(const StringName& p_param) const;

    virtual void set_code(const String& p_Code) = 0;
    virtual bool is_animated() const = 0;
    virtual bool casts_shadows() const = 0;
    virtual RS::ShaderNativeSourceCode get_native_source_code() const { return RS::ShaderNativeSourceCode(); }

    virtual ~ShaderData() {}
  };
  struct Material;
  struct Shader {
	ShaderData *data = nullptr;
	String code;
	String path_hint;
	ShaderType type;
	HashMap<StringName, HashMap<int, RID>> default_texture_parameter;
	HashSet<Material *> owners;
  };
  mutable RID_Owner<Shader, true> shader_owner;

 public:

  /* GLOBAL SHADER UNIFORM API */
  virtual void global_shader_parameter_add(const StringName& p_name, RS::GlobalShaderParameterType p_type, const Variant& p_value) override;
  virtual void global_shader_parameter_remove(const StringName& p_name) override;
  virtual Vector<StringName> global_shader_parameter_get_list() const override;

  virtual void global_shader_parameter_set(const StringName& p_name, const Variant& p_value) override;
  virtual void global_shader_parameter_set_override(const StringName& p_name, const Variant& p_value) override;
  virtual Variant global_shader_parameter_get(const StringName& p_name) const override;
  virtual RS::GlobalShaderParameterType global_shader_parameter_get_type(const StringName& p_name) const override;

  virtual void global_shader_parameters_load_settings(bool p_load_textures = true) override;
  virtual void global_shader_parameters_clear() override;

  virtual int32_t global_shader_parameters_instance_allocate(RID p_instance) override;
  virtual void global_shader_parameters_instance_free(RID p_instance) override;
  virtual void global_shader_parameters_instance_update(RID p_instance, int p_index, const Variant& p_value, int p_flags_count = 0) override;

  /* SHADER API */
  virtual RID shader_allocate() override { return shader_owner.allocate_rid(); }
  virtual void shader_initialize(RID p_rid) override;
  virtual void shader_free(RID p_rid) override;

  virtual void shader_set_code(RID p_shader, const String& p_code) override;
  virtual void shader_set_path_hint(RID p_shader, const String& p_path) override;
  virtual String shader_get_code(RID p_shader) const override;
  virtual void get_shader_parameter_list(RID p_shader, List<PropertyInfo>* p_param_list) const override;

  virtual void shader_set_default_texture_parameter(RID p_shader, const StringName& p_name, RID p_texture, int p_index) override;
  virtual RID shader_get_default_texture_parameter(RID p_shader, const StringName& p_name, int p_index) const override;
  virtual Variant shader_get_parameter_default(RID p_material, const StringName& p_param) const override;

  virtual RS::ShaderNativeSourceCode shader_get_native_source_code(RID p_shader) const override;

  /* MATERIAL API */

  virtual RID material_allocate() override;
  virtual void material_initialize(RID p_rid) override;
  virtual void material_free(RID p_rid) override;

  virtual void material_set_render_priority(RID p_material, int priority) override;
  virtual void material_set_shader(RID p_shader_material, RID p_shader) override;

  virtual void material_set_param(RID p_material, const StringName& p_param, const Variant& p_value) override;
  virtual Variant material_get_param(RID p_material, const StringName& p_param) const override;

  virtual void material_set_next_pass(RID p_material, RID p_next_material) override;

  virtual bool material_is_animated(RID p_material) override;
  virtual bool material_casts_shadows(RID p_material) override;
  virtual void material_get_instance_shader_parameters(RID p_material, List<InstanceShaderParam>* r_parameters) override;

  virtual void material_update_dependency(RID p_material, DependencyTracker* p_instance) override;
};
}  // namespace lain::RendererRD

#endif  // MATERIAL_STORAGE_H
