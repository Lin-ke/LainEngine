#include "shader.h"
#include "function/render/rendering_system/rendering_system.h"
using namespace lain;

Shader::Mode Shader::get_mode() const {
  return mode;
}
void lain::Shader::set_path(const String& p_path, bool p_take_over) {
  Resource::set_path(p_path, p_take_over);
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
