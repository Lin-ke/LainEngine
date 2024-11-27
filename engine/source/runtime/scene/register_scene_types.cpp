#include "core/meta/serializer/serializer.h"
// scene
#include "scene/resources/io/resource_format_text.h"
#include "scene/resources/io/resource_format_shader.h"
#include "scene/resources/common/shader_include.h"
#include "register_scene_types.h"

// function types
#include "function/shader/shader_types.h"

namespace lain{
	static Ref<ResourceFormatLoaderText> resource_format_loader_text;
	static Ref<ResourceFormatSaverText> resource_format_saver_text;
	static Ref<ResourceFormatLoaderShader> resource_format_loader_shader;
	static Ref<ResourceFormatSaverShader> resource_format_saver_shader;
	static Ref<ResourceFormatLoaderShaderInclude> resource_format_loader_shader_include;
	static Ref<ResourceFormatSaverShaderInclude> resource_format_saver_shader_include;
	

  void register_scene_types(){
    resource_format_loader_text.instantiate();
		ResourceLoader::add_resource_format_loader(resource_format_loader_text);


		resource_format_saver_text.instantiate();
		ResourceSaver::add_resource_format_saver(resource_format_saver_text);

		resource_format_loader_shader.instantiate();
		ResourceLoader::add_resource_format_loader(resource_format_loader_shader);

		resource_format_saver_shader.instantiate();
		ResourceSaver::add_resource_format_saver(resource_format_saver_shader);

		resource_format_loader_shader_include.instantiate();
		ResourceLoader::add_resource_format_loader(resource_format_loader_shader_include);

		resource_format_saver_shader_include.instantiate();
		ResourceSaver::add_resource_format_saver(resource_format_saver_shader_include);

		auto& importers = ResourceLoader::ext_to_loader_idx;
		// L_JSON(importers);
		
  }
	shader::ShaderTypes *shader_types = nullptr;

  void register_system_types() {
		shader_types = memnew(shader::ShaderTypes);
	}

  }  // namespace lain