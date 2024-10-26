#include "shader.h"
#include "shader_include.h"
#include "function/render/rendering_system/rendering_system.h"
#include "function/shader/shader_language.h"
#include "function/shader/shader_preprocessor.h"
namespace lain{

Shader::Mode Shader::get_mode() const {
  return mode;
}
void Shader::set_path(const String& p_path, bool p_take_over) {
  Resource::set_path(p_path, p_take_over);
}
void Shader::set_code(const String& p_code) {
  // for (const Ref<ShaderInclude> &E : include_dependencies) {
	// 	E->disconnect_changed(callable_mp(this, &Shader::_dependency_changed));
	// }

	code = p_code;
	String pp_code = p_code;

	{
		String path = GetPath();
		if (path.is_empty()) {
			path = include_path;
		}
		// Preprocessor must run here and not in the server because:
		// 1) Need to keep track of include dependencies at resource level
		// 2) Server does not do interaction with Resource filetypes, this is a scene level feature.
		HashSet<Ref<ShaderInclude>> new_include_dependencies;
		shader::ShaderPreprocessor preprocessor;
		Error result = preprocessor.preprocess(p_code, path, pp_code, nullptr, nullptr, nullptr, &new_include_dependencies);
		if (result == OK) {
			// This ensures previous include resources are not freed and then re-loaded during parse (which would make compiling slower)
			include_dependencies = new_include_dependencies;
		}
	}

	// Try to get the shader type from the final, fully preprocessed shader code.
	String type = shader::ShaderLanguage::get_shader_type(pp_code);

	if (type == "canvas_item") {
		mode = MODE_CANVAS_ITEM;
	} else if (type == "particles") {
		mode = MODE_PARTICLES;
	} else if (type == "sky") {
		mode = MODE_SKY;
	} else if (type == "fog") {
		mode = MODE_FOG;
	} else {
		mode = MODE_SPATIAL;
	}

	for (const Ref<ShaderInclude> &E : include_dependencies) {
		// E->connect_changed(callable_mp(this, &Shader::_dependency_changed));
	}

	RenderingSystem::get_singleton()->shader_set_code(shader, pp_code);

	// emit_changed();
}
String Shader::get_code() const {
  return code;
}
Shader::Shader() {
	shader = RenderingSystem::get_singleton()->shader_create();
}
Shader::~Shader() {
	ERR_FAIL_NULL(RenderingSystem::get_singleton());
	RenderingSystem::get_singleton()->free(shader);
}
	
}