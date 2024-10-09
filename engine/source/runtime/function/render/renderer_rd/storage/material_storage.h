#ifndef MATERIAL_STORAGE_RDIMPL_H
#define MATERIAL_STORAGE_RDIMPL_H
//
#include "function/render/rendering_system/material_storage_api.h"
#include "texture_storage.h"
namespace lain::RendererRD {

class MaterialStorage : public RendererMaterialStorage {
 private:
  static MaterialStorage* p_singleton;

 public:
  enum ShaderType { SHADER_TYPE_2D, SHADER_TYPE_3D, SHADER_TYPE_PARTICLES, SHADER_TYPE_SKY, SHADER_TYPE_FOG, SHADER_TYPE_MAX };
  MaterialStorage();
  ~MaterialStorage() override;
  // ShaderData
  // 这里很复杂，@？ 如何与shader的反射相结合
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
    ShaderData* data = nullptr;
    String code;
    String path_hint;
    ShaderType type;
    HashMap<StringName, HashMap<int, RID>> default_texture_parameter;
    HashSet<Material*> owners;
  };
  
  mutable RID_Owner<Shader, true> shader_owner;

  
	struct MaterialData {
		Vector<RendererRD::TextureStorage::RenderTarget *> render_target_cache;
		void update_uniform_buffer(const HashMap<StringName, ShaderLanguage::ShaderNode::Uniform> &p_uniforms, const uint32_t *p_uniform_offsets, const HashMap<StringName, Variant> &p_parameters, uint8_t *p_buffer, uint32_t p_buffer_size, bool p_use_linear_color);
		void update_textures(const HashMap<StringName, Variant> &p_parameters, const HashMap<StringName, HashMap<int, RID>> &p_default_textures, const Vector<ShaderCompiler::GeneratedCode::Texture> &p_texture_uniforms, RID *p_textures, bool p_use_linear_color, bool p_3d_material);
		void set_as_used();

		virtual void set_render_priority(int p_priority) = 0;
		virtual void set_next_pass(RID p_pass) = 0;
		virtual bool update_parameters(const HashMap<StringName, Variant> &p_parameters, bool p_uniform_dirty, bool p_textures_dirty) = 0;
		virtual ~MaterialData();

		//to be used internally by update_parameters, in the most common configuration of material parameters
		bool update_parameters_uniform_set(const HashMap<StringName, Variant> &p_parameters, bool p_uniform_dirty, bool p_textures_dirty, const HashMap<StringName, ShaderLanguage::ShaderNode::Uniform> &p_uniforms, const uint32_t *p_uniform_offsets, const Vector<ShaderCompiler::GeneratedCode::Texture> &p_texture_uniforms, const HashMap<StringName, HashMap<int, RID>> &p_default_texture_params, uint32_t p_ubo_size, RID &r_uniform_set, RID p_shader, uint32_t p_shader_uniform_set, bool p_use_linear_color, bool p_3d_material);
		void free_parameters_uniform_set(RID p_uniform_set);

	private:
		friend class MaterialStorage;

		RID self;
		List<RID>::Element *global_buffer_E = nullptr;
		List<RID>::Element *global_texture_E = nullptr;
		uint64_t global_textures_pass = 0;
		HashMap<StringName, uint64_t> used_global_textures;

		//internally by update_parameters_uniform_set
		Vector<uint8_t> ubo_data[2]; // 0: linear buffer; 1: sRGB buffer.
		RID uniform_buffer[2]; // 0: linear buffer; 1: sRGB buffer.
		Vector<RID> texture_cache;
	};

 public:
  /* GLOBAL SHADER UNIFORM API */
  // 为所有的着色器设置全局参数 @todo
  // virtual void global_shader_parameter_add(const StringName& p_name, RS::GlobalShaderParameterType p_type, const Variant& p_value) override;
  // virtual void global_shader_parameter_remove(const StringName& p_name) override;
  // virtual Vector<StringName> global_shader_parameter_get_list() const override;

  // virtual void global_shader_parameter_set(const StringName& p_name, const Variant& p_value) override;
  // virtual void global_shader_parameter_set_override(const StringName& p_name, const Variant& p_value) override;
  // virtual Variant global_shader_parameter_get(const StringName& p_name) const override;
  // virtual RS::GlobalShaderParameterType global_shader_parameter_get_type(const StringName& p_name) const override;

  // virtual void global_shader_parameters_load_settings(bool p_load_textures = true) override;
  // virtual void global_shader_parameters_clear() override;

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
