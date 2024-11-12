#ifndef MATERIAL_STORAGE_RDIMPL_H
#define MATERIAL_STORAGE_RDIMPL_H
//
#include "function/render/rendering_system/material_storage_api.h"
#include "texture_storage.h"
#include "function/shader/shader_language.h"
#include "function/shader/shader_compiler.h"
namespace lain::RendererRD {

class MaterialStorage : public RendererMaterialStorage {
 private:
  static MaterialStorage* p_singleton;

 public:
	bool owns_shader(RID p_rid) {
		return shader_owner.owns(p_rid);
	}
	bool owns_material(RID p_rid){
		return material_owner.owns(p_rid);
	}
	bool free(RID); 
	static MaterialStorage* get_singleton() { return p_singleton; }
  enum ShaderType { SHADER_TYPE_2D, SHADER_TYPE_3D, SHADER_TYPE_PARTICLES, SHADER_TYPE_SKY, SHADER_TYPE_FOG, SHADER_TYPE_MAX };
  MaterialStorage();
  ~MaterialStorage() override;
  // ShaderData
  // 这里很复杂，@？ 如何与shader的反射相结合
  struct ShaderData {
    String path;
    HashMap<StringName, shader::ShaderLanguage::ShaderNode::Uniform> uniforms; // 这些在setcode 的时候编译好
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
    HashSet<Material*> owners; // 这个shader 管着这些材质
  };
	Shader *get_shader(RID p_rid) { return shader_owner.get_or_null(p_rid); }
  
  mutable RID_Owner<Shader, true> shader_owner;
	typedef ShaderData *(*ShaderDataRequestFunction)();
	ShaderDataRequestFunction shader_data_request_func[SHADER_TYPE_MAX];
  
	struct MaterialData {
		Vector<RendererRD::TextureStorage::RenderTarget *> render_target_cache;
		void update_uniform_buffer(const HashMap<StringName, shader::ShaderLanguage::ShaderNode::Uniform> &p_uniforms, const uint32_t *p_uniform_offsets, const HashMap<StringName, Variant> &p_parameters, uint8_t *p_buffer, uint32_t p_buffer_size, bool p_use_linear_color);
		void update_textures(const HashMap<StringName, Variant> &p_parameters, const HashMap<StringName, HashMap<int, RID>> &p_default_textures, const Vector<shader::ShaderCompiler::GeneratedCode::Texture> &p_texture_uniforms, RID *p_textures, bool p_use_linear_color, bool p_3d_material);
		void set_as_used();

		virtual void set_render_priority(int p_priority) = 0;
		virtual void set_next_pass(RID p_pass) = 0;
		virtual bool update_parameters(const HashMap<StringName, Variant> &p_parameters, bool p_uniform_dirty, bool p_textures_dirty) = 0;
		virtual ~MaterialData();

		//to be used internally by update_parameters, in the most common configuration of material parameters
		bool update_parameters_uniform_set(const HashMap<StringName, Variant> &p_parameters, bool p_uniform_dirty, bool p_textures_dirty, const HashMap<StringName, shader::ShaderLanguage::ShaderNode::Uniform> &p_uniforms, const uint32_t *p_uniform_offsets, const Vector<shader::ShaderCompiler::GeneratedCode::Texture> &p_texture_uniforms, const HashMap<StringName, HashMap<int, RID>> &p_default_texture_params, uint32_t p_ubo_size, RID &r_uniform_set, RID p_shader, uint32_t p_shader_uniform_set, bool p_use_linear_color, bool p_3d_material);
		void free_parameters_uniform_set(RID p_uniform_set);

	private:
		friend class MaterialStorage;

		RID self; // 与 Material.self 一致 (RID)
		List<RID>::Element *global_buffer_E = nullptr;
		List<RID>::Element *global_texture_E = nullptr;
		uint64_t global_textures_pass = 0;
		HashMap<StringName, uint64_t> used_global_textures;

		//internally by update_parameters_uniform_set
		Vector<uint8_t> ubo_data[2]; // 0: linear buffer; 1: sRGB buffer.
		RID uniform_buffer[2]; // 0: linear buffer; 1: sRGB buffer.
		Vector<RID> texture_cache;
	};

	typedef MaterialData *(*MaterialDataRequestFunction)(ShaderData *);
  
	struct Material {
		RID self;
		MaterialData *data = nullptr;
		Shader *shader = nullptr;
		//shortcut to shader data and type
		ShaderType shader_type = SHADER_TYPE_MAX;
		uint32_t shader_id = 0; // shader.get_local_index
		bool uniform_dirty = false;
		bool texture_dirty = false;
		HashMap<StringName, Variant> params;
		int32_t priority = 0;
		RID next_pass; // ? next material
		SelfList<Material> update_element; // 更新链表的结点

		Dependency dependency;

		Material() :
				update_element(this) {}
	};
	mutable RID_Owner<Material, true> material_owner;
	// 由于MaterialData 是一个接口类，利用下面的方式获得实际的函数（根据shaderdata*，返回materialdata*)(scene_shader_forward_clustered.cpp)
	MaterialDataRequestFunction material_data_request_func[SHADER_TYPE_MAX];

	void material_set_data_request_function(ShaderType p_shader_type, MaterialDataRequestFunction p_function);
	MaterialDataRequestFunction material_get_data_request_function(ShaderType p_shader_type);


		struct Samplers {
		RID rids[RS::CANVAS_ITEM_TEXTURE_FILTER_MAX][RS::CANVAS_ITEM_TEXTURE_REPEAT_MAX];
		float mipmap_bias = 0.0f;
		bool use_nearest_mipmap_filter = false; // global get
		int anisotropic_filtering_level = 2;

		_FORCE_INLINE_ RID get_sampler(RS::CanvasItemTextureFilter p_filter, RS::CanvasItemTextureRepeat p_repeat) const {
			return rids[p_filter][p_repeat];
		}

		Vector<RD::Uniform> get_uniforms(int p_first_index) const;
		bool is_valid() const;
		bool is_null() const;
	};
	/* GLOBAL SHADER UNIFORM API */

	struct GlobalShaderUniforms {
		enum {
			BUFFER_DIRTY_REGION_SIZE = 1024
		};
		struct Variable {
			HashSet<RID> texture_materials; // materials using this

			RS::GlobalShaderParameterType type;
			Variant value;
			Variant override;
			int32_t buffer_index; //for vectors
			int32_t buffer_elements; //for vectors
		};

		HashMap<StringName, Variable> variables;

		struct Value {
			float x;
			float y;
			float z;
			float w;
		};

		struct ValueInt {
			int32_t x;
			int32_t y;
			int32_t z;
			int32_t w;
		};

		struct ValueUInt {
			uint32_t x;
			uint32_t y;
			uint32_t z;
			uint32_t w;
		};

		struct ValueUsage {
			uint32_t elements = 0;
		};

		List<RID> materials_using_buffer;
		List<RID> materials_using_texture;

		RID buffer;
		Value *buffer_values = nullptr;
		ValueUsage *buffer_usage = nullptr; //  elements记录了有几个
		bool *buffer_dirty_regions = nullptr;
		uint32_t buffer_dirty_region_count = 0;

		uint32_t buffer_size;

		bool must_update_texture_materials = false;
		bool must_update_buffer_materials = false;

		HashMap<RID, int32_t> instance_buffer_pos;
	} global_shader_uniforms;
	// 找到一个可容纳p_elements的buffer
	int32_t _global_shader_uniform_allocate(uint32_t p_elements);
	void _global_shader_uniform_store_in_buffer(int32_t p_index, RS::GlobalShaderParameterType p_type, const Variant &p_value);
	void _global_shader_uniform_mark_buffer_dirty(int32_t p_index, int32_t p_elements);
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
	// 这里会直接删除对应material 的 data，会引起野指针的问题吗？
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
	// 清理原来的data，设置新的data
  virtual void material_set_shader(RID p_shader_material, RID p_shader) override;

  virtual void material_set_param(RID p_material, const StringName& p_param, const Variant& p_value) override;
  virtual Variant material_get_param(RID p_material, const StringName& p_param) const override;

  virtual void material_set_next_pass(RID p_material, RID p_next_material) override;

  virtual bool material_is_animated(RID p_material) override;
  virtual bool material_casts_shadows(RID p_material) override;
  virtual void material_get_instance_shader_parameters(RID p_material, List<InstanceShaderParam>* r_parameters) override;

  virtual void material_update_dependency(RID p_material, DependencyTracker* p_instance) override;

	/* Samplers */
	Samplers default_samplers;

	Samplers samplers_rd_allocate(float p_mipmap_bias = 0.0f) const;
	void samplers_rd_free(Samplers &p_samplers) const;

	_FORCE_INLINE_ RID sampler_rd_get_default(RS::CanvasItemTextureFilter p_filter, RS::CanvasItemTextureRepeat p_repeat) {
		return default_samplers.get_sampler(p_filter, p_repeat);
	}

	_FORCE_INLINE_ const Samplers &samplers_rd_get_default() const {
		return default_samplers;
	}


  // helpers

  static _FORCE_INLINE_ void store_camera(const Projection &p_mtx, float *p_array) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				p_array[i * 4 + j] = p_mtx.columns[i][j];
			}
		}
	}
  // 软阴影用的滤波的核 @todo 找到shader的位置
	static _FORCE_INLINE_ void store_soft_shadow_kernel(const float *p_kernel, float *p_array) {
		for (int i = 0; i < 128; i++) {
			p_array[i] = p_kernel[i];
		}
	}
  	static _FORCE_INLINE_ void store_transform(const Transform3D &p_mtx, float *p_array) {
		p_array[0] = p_mtx.basis.rows[0][0];
		p_array[1] = p_mtx.basis.rows[1][0];
		p_array[2] = p_mtx.basis.rows[2][0];
		p_array[3] = 0;
		p_array[4] = p_mtx.basis.rows[0][1];
		p_array[5] = p_mtx.basis.rows[1][1];
		p_array[6] = p_mtx.basis.rows[2][1];
		p_array[7] = 0;
		p_array[8] = p_mtx.basis.rows[0][2];
		p_array[9] = p_mtx.basis.rows[1][2];
		p_array[10] = p_mtx.basis.rows[2][2];
		p_array[11] = 0;
		p_array[12] = p_mtx.origin.x;
		p_array[13] = p_mtx.origin.y;
		p_array[14] = p_mtx.origin.z;
		p_array[15] = 1;
	}

  
	static _FORCE_INLINE_ void store_transform_3x3(const Basis &p_mtx, float *p_array) {
		p_array[0] = p_mtx.rows[0][0];
		p_array[1] = p_mtx.rows[1][0];
		p_array[2] = p_mtx.rows[2][0];
		p_array[3] = 0;
		p_array[4] = p_mtx.rows[0][1];
		p_array[5] = p_mtx.rows[1][1];
		p_array[6] = p_mtx.rows[2][1];
		p_array[7] = 0;
		p_array[8] = p_mtx.rows[0][2];
		p_array[9] = p_mtx.rows[1][2];
		p_array[10] = p_mtx.rows[2][2];
		p_array[11] = 0;
	}

	_FORCE_INLINE_ MaterialData *material_get_data(RID p_material, ShaderType p_shader_type) {
		Material *material = material_owner.get_or_null(p_material);
		if (!material || material->shader_type != p_shader_type) {
			return nullptr;
		} else {
			return material->data;
		}
	}



	void shader_set_data_request_function(ShaderType p_shader_type, ShaderDataRequestFunction p_function);


  private:
	SelfList<Material>::List material_update_list;
  void _material_queue_update(Material *material, bool p_uniform, bool p_texture);
};
}  // namespace lain::RendererRD

#endif  // MATERIAL_STORAGE_H
