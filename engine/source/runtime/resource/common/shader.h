#pragma once
#ifndef __SHADER__H__
#define __SHADER__H__
#include "core/io/resource.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "resource/common/texture.h"

namespace lain {
	// Shader -> Shader compiler -> Uber shader -> Compile to SPRIV

	class Shader : public Resource {
		LCLASS(Shader, Resource);

	public:
		enum Mode {
			MODE_SPATIAL,
			MODE_CANVAS_ITEM,
			MODE_PARTICLES,
			MODE_SKY,
			MODE_FOG,
			MODE_MAX
		};
		/*
		

    spatial for 3D rendering.

    canvas_item for 2D rendering.

    particles for particle systems.

    sky to render Skies.

    fog to render FogVolumes
	*/
	protected:
		static void _bind_methods();

	public:
		RID shader;
		Mode mode = MODE_SPATIAL; // 我感觉这不是一个好设计
		//HashSet<Ref<ShaderInclude>> include_dependencies;
		String code;
		String include_path;


		//void set_mode(Mode p_mode);
		virtual Mode get_mode() const;

		virtual void SetPath(const String& p_path, bool p_take_over = false) override;
		L_INLINE void set_include_path(const String& p_path) { include_path = p_path; }

		L_INLINE void set_code(const String& p_code) { code = p_code; }
		String get_code() const;

		void get_shader_uniform_list(List<PropertyInfo>* p_params, bool p_get_groups = false) const;

		void set_default_texture_parameter(const StringName& p_name, const Ref<Texture2D>& p_texture, int p_index = 0);
		Ref<Texture2D> get_default_texture_parameter(const StringName& p_name, int p_index = 0) const;
		void get_default_texture_parameter_list(List<StringName>* r_textures) const;

		virtual bool is_text_shader() const;

		virtual RID GetRID() const override;

		Shader();
		~Shader();

	};

	class ResourceFormatLoaderShader : public ResourceFormatLoader {
	public:
		virtual Ref<Resource> load(const String& p_path, const String& p_original_path = "", Error* r_error = nullptr, bool p_use_sub_threads = false, float* r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
		virtual void get_possible_extensions(List<String>* p_extensions) const {
			p_extensions->push_back("vs");
			p_extensions->push_back("fs");
			p_extensions->push_back("cs"); // compute shader
		}
	};

	class ResourceFormatSaverShader : public ResourceFormatSaver {
	public:
		virtual Error save(const Ref<Resource>& p_resource, const String& p_path, uint32_t p_flags = 0) override;
		//virtual void get_recognized_extensions(const Ref<Resource>& p_resource, List<String>* p_extensions) const override;
		
		virtual void get_possible_extensions(List<String>* p_extensions) const {
			p_extensions->push_back("vs");
			p_extensions->push_back("fs");
			p_extensions->push_back("cs"); // compute shader

		}
		virtual void get_possible_resources(List<String>* p_extensions) const {
			p_extensions->push_back("Shader");
		}
	};
}
#endif