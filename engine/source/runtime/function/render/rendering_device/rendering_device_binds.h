#ifndef RENDERING_DEVICE_BINDS_H
#define RENDERING_DEVICE_BINDS_H
// 向上级 (Editor、脚本) 暴露的接口
// 这个值得学习的是封装了很底层的操作，比如load一个glsl
// 和 RD::TextureFormat 这种底层的数据结构
#include "core/object/refcounted.h"
#include "rendering_device.h"
#include "core/io/resource.h"

#define RD_SETGET(m_type, m_member)          \
  void set_##m_member(m_type p_##m_member) { \
    base.m_member = p_##m_member;            \
  }                                          \
  m_type get_##m_member() const {            \
    return base.m_member;                    \
  }

#define RD_SETGET_SUB(m_type, m_sub, m_member)         \
  void set_##m_sub##_##m_member(m_type p_##m_member) { \
    base.m_sub.m_member = p_##m_member;                \
  }                                                    \
  m_type get_##m_sub##_##m_member() const {            \
    return base.m_sub.m_member;                        \
  }
namespace lain {

class RDTextureFormat : public RefCounted {
  LCLASS(RDTextureFormat, RefCounted)

  friend class RenderingDevice;
  friend class RenderSceneBuffersRD;

  RD::TextureFormat base;

 public:
  RD_SETGET(RD::DataFormat, format)
  RD_SETGET(uint32_t, width)
  RD_SETGET(uint32_t, height)
  RD_SETGET(uint32_t, depth)
  RD_SETGET(uint32_t, array_layers)
  RD_SETGET(uint32_t, mipmaps)
  RD_SETGET(RD::TextureType, texture_type)
  RD_SETGET(RD::TextureSamples, samples)
  RD_SETGET(BitField<RenderingDevice::TextureUsageBits>, usage_bits)

  void add_shareable_format(RD::DataFormat p_format) { base.shareable_formats.push_back(p_format); }
  void remove_shareable_format(RD::DataFormat p_format) { base.shareable_formats.erase(p_format); }
};


class RDTextureView : public RefCounted {
	LCLASS(RDTextureView, RefCounted)

	friend class RenderingDevice;
	friend class RenderSceneBuffersRD;

	RD::TextureView base;

public:
	RD_SETGET(RD::DataFormat, format_override)
	RD_SETGET(RD::TextureSwizzle, swizzle_r)
	RD_SETGET(RD::TextureSwizzle, swizzle_g)
	RD_SETGET(RD::TextureSwizzle, swizzle_b)
	RD_SETGET(RD::TextureSwizzle, swizzle_a)
protected:
	
};


class RDFramebufferPass : public RefCounted {
	LCLASS(RDFramebufferPass, RefCounted)
	friend class RenderingDevice;
	friend class FramebufferCacheRD;

	RD::FramebufferPass base;

public:
	RD_SETGET(PackedInt32Array, color_attachments)
	RD_SETGET(PackedInt32Array, input_attachments)
	RD_SETGET(PackedInt32Array, resolve_attachments)
	RD_SETGET(PackedInt32Array, preserve_attachments)
	RD_SETGET(int32_t, depth_attachment)
protected:
	enum {
		ATTACHMENT_UNUSED = -1
	};

};

class RDShaderSource : public RefCounted {
	LCLASS(RDShaderSource, RefCounted)
	String source[RD::SHADER_STAGE_MAX];
	RD::ShaderLanguage language = RD::SHADER_LANGUAGE_GLSL;

public:
	void set_stage_source(RD::ShaderStage p_stage, const String &p_source) {
		ERR_FAIL_INDEX(p_stage, RD::SHADER_STAGE_MAX);
		source[p_stage] = p_source;
	}

	String get_stage_source(RD::ShaderStage p_stage) const {
		ERR_FAIL_INDEX_V(p_stage, RD::SHADER_STAGE_MAX, String());
		return source[p_stage];
	}

	void set_language(RD::ShaderLanguage p_language) {
		language = p_language;
	}

	RD::ShaderLanguage get_language() const {
		return language;
	}
};

class RDShaderSPIRV : public Resource {
	LCLASS(RDShaderSPIRV, Resource)

	Vector<uint8_t> bytecode[RD::SHADER_STAGE_MAX];
	String compile_error[RD::SHADER_STAGE_MAX];

public:
	void set_stage_bytecode(RD::ShaderStage p_stage, const Vector<uint8_t> &p_bytecode) {
		ERR_FAIL_INDEX(p_stage, RD::SHADER_STAGE_MAX);
		bytecode[p_stage] = p_bytecode;
	}

	Vector<uint8_t> get_stage_bytecode(RD::ShaderStage p_stage) const {
		ERR_FAIL_INDEX_V(p_stage, RD::SHADER_STAGE_MAX, Vector<uint8_t>());
		return bytecode[p_stage];
	}

	Vector<RD::ShaderStageSPIRVData> get_stages() const {
		Vector<RD::ShaderStageSPIRVData> stages;
		for (int i = 0; i < RD::SHADER_STAGE_MAX; i++) {
			if (bytecode[i].size()) {
				RD::ShaderStageSPIRVData stage;
				stage.shader_stage = RD::ShaderStage(i);
				stage.spirv = bytecode[i];
				stages.push_back(stage);
			}
		}
		return stages;
	}

	void set_stage_compile_error(RD::ShaderStage p_stage, const String &p_compile_error) {
		ERR_FAIL_INDEX(p_stage, RD::SHADER_STAGE_MAX);
		compile_error[p_stage] = p_compile_error;
	}

	String get_stage_compile_error(RD::ShaderStage p_stage) const {
		ERR_FAIL_INDEX_V(p_stage, RD::SHADER_STAGE_MAX, String());
		return compile_error[p_stage];
	}
  };

class RDShaderFile : public Resource {
	LCLASS(RDShaderFile, Resource)

	HashMap<StringName, Ref<RDShaderSPIRV>> versions;
	String base_error;

public:
	void set_bytecode(const Ref<RDShaderSPIRV> &p_bytecode, const StringName &p_version = StringName()) {
		ERR_FAIL_COND(p_bytecode.is_null());
		versions[p_version] = p_bytecode;
		// emit_changed();
	}

	Ref<RDShaderSPIRV> get_spirv(const StringName &p_version = StringName()) const {
		ERR_FAIL_COND_V(!versions.has(p_version), Ref<RDShaderSPIRV>());
		return versions[p_version];
	}

	Vector<RD::ShaderStageSPIRVData> get_spirv_stages(const StringName &p_version = StringName()) const {
		ERR_FAIL_COND_V(!versions.has(p_version), Vector<RD::ShaderStageSPIRVData>());
		return versions[p_version]->get_stages();
	}

	Array get_version_list() const {
		Vector<StringName> vnames;
		for (const KeyValue<StringName, Ref<RDShaderSPIRV>> &E : versions) {
			vnames.push_back(E.key);
		}
		vnames.sort_custom<StringName::AlphCompare>();
		Array ret;
		ret.resize(vnames.size());
		for (int i = 0; i < vnames.size(); i++) {
			ret[i] = vnames[i];
		}
		return ret;
	}

	void set_base_error(const String &p_error) {
		base_error = p_error;
		// emit_changed();
	}

	String get_base_error() const {
		return base_error;
	}

	void print_errors(const String &p_file) {
		if (!base_error.is_empty()) {
			ERR_PRINT("Error parsing shader '" + p_file + "':\n\n" + base_error);
		} else {
			for (KeyValue<StringName, Ref<RDShaderSPIRV>> &E : versions) {
				for (int i = 0; i < RD::SHADER_STAGE_MAX; i++) {
					String error = E.value->get_stage_compile_error(RD::ShaderStage(i));
					if (!error.is_empty()) {
						static const char *stage_str[RD::SHADER_STAGE_MAX] = {
							"vertex",
							"fragment",
							"tesselation_control",
							"tesselation_evaluation",
							"compute"
						};

						ERR_PRINT("Error parsing shader '" + p_file + "', version '" + String(E.key) + "', stage '" + stage_str[i] + "':\n\n" + error);
					}
				}
			}
		}
	}

	typedef String (*OpenIncludeFunction)(const String &, void *userdata);
	Error parse_versions_from_text(const String &p_text, const String p_defines = String(), OpenIncludeFunction p_include_func = nullptr, void *p_include_func_userdata = nullptr);

protected:
	Dictionary _get_versions() const {
		Array vnames = get_version_list();
		Dictionary ret;
		for (int i = 0; i < vnames.size(); i++) {
			ret[vnames[i]] = versions[vnames[i]];
		}
		return ret;
	}
	void _set_versions(const Dictionary &p_versions) {
		versions.clear();
		List<Variant> keys;
		p_versions.get_key_list(&keys);
		for (const Variant &E : keys) {
			StringName vname = E;
			Ref<RDShaderSPIRV> bc = p_versions[E];
			ERR_CONTINUE(bc.is_null());
			versions[vname] = bc;
		}

		// emit_changed();
	}
};
}  // namespace lain

#endif