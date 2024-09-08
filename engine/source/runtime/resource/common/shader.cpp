#include "shader.h";
using namespace lain;
void lain::Shader::set_path(const String& p_path, bool p_take_over) {
	Resource::set_path(p_path, p_take_over);

}
String Shader::get_code() const {
  return code;
}
