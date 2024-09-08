#include "resource_format_shader.h"
#include "resource/common/shader.h"
using namespace lain;

Ref<Resource> ResourceFormatLoaderShader::load(const String& p_path, const String& p_original_path, Error* r_error, bool p_use_sub_threads, float* r_progress,
                                               CacheMode p_cache_mode) {
  if (r_error) {
    *r_error = ERR_FILE_CANT_OPEN;
  }

  Error error = OK;
  Vector<uint8_t> buffer = FileAccess::get_file_as_bytes(p_path, &error);
  ERR_FAIL_COND_V_MSG(error, nullptr, "Cannot load shader: " + p_path);

  String str;
  if (buffer.size() > 0) {
    error = str.parse_utf8((const char*)buffer.ptr(), buffer.size());
    ERR_FAIL_COND_V_MSG(error, nullptr, "Cannot parse shader: " + p_path);
  }

  Ref<Shader> shader;
  shader.instantiate();

  shader->set_include_path(p_path);
  shader->set_code(str);

  if (r_error) {
    *r_error = OK;
  }

  return shader;
}

void lain::ResourceFormatLoaderShader::get_recognized_extensions(List<String>* p_extensions) const {
    p_extensions->push_back("lshader");

}

void lain::ResourceFormatLoaderShader::get_recognized_resources(List<String>* p_resource_class) const {
    p_resource_class->push_back("Shader");
}

Error lain::ResourceFormatSaverShader::save(const Ref<Resource>& p_resource, const String& p_path, uint32_t p_flags) {
  	Ref<Shader> shader = p_resource;
	ERR_FAIL_COND_V(shader.is_null(), ERR_INVALID_PARAMETER);

	String source = shader->get_code();

	Error err;
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE, &err);

	ERR_FAIL_COND_V_MSG(err, err, "Cannot save shader '" + p_path + "'.");

	file->store_string(source);
	if (file->get_error() != OK && file->get_error() != ERR_FILE_EOF) {
		return ERR_CANT_CREATE;
	}

	return OK;
}

void lain::ResourceFormatSaverShader::get_recognized_extensions(const Ref<Resource>& p_resource, List<String>* p_extensions) const {
  if (const Shader* shader = Object::cast_to<Shader>(*p_resource)) {
    if (shader->is_text_shader()) {
      p_extensions->push_back("lshader");
    }
  }
}

void lain::ResourceFormatSaverShader::get_recognized_extensions(List<String>* p_extensions) const {
    p_extensions->push_back("lshader");
}

void lain::ResourceFormatSaverShader::get_recognized_resources(List<String>* p_resource_class) const {
    p_resource_class->push_back("Shader");
}
