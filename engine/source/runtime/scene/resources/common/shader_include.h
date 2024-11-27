#ifndef SHADER_INCLUDE_H
#define SHADER_INCLUDE_H

#include "core/io/resource.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/templates/hash_set.h"
#include "core/meta/serializer/serializer.h"

namespace lain{

class ShaderInclude : public Resource {
	LCLASS(ShaderInclude, Resource);
	OBJ_SAVE_TYPE(ShaderInclude);

private:
	String code;
	String include_path;
	HashSet<Ref<ShaderInclude>> include_dependencies; // 依赖的其他Include
	void _dependency_changed();

protected:
	static void _bind_methods();

public:
	void set_code(const String &p_text);
	String get_code() const;

	void set_include_path(const String &p_path);
};

class ResourceFormatLoaderShaderInclude : public ResourceFormatLoader {
public:
	virtual Ref<Resource> load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_use_sub_threads = false, float *r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
	virtual void get_recognized_extensions(List<String> *p_extensions) const override;
	virtual void get_recognized_resources(List<String> *p_resources) const override;
	virtual bool handles_type(const String &p_type) const override;
	virtual String get_resource_type(const String &p_path) const override;
};

class ResourceFormatSaverShaderInclude : public ResourceFormatSaver {
public:
	virtual Error save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags = 0) override;
	// 下面的函数多判断一次 p_resource 的类型是否被支持，支持则添加拓展名到 p_extensions中
	virtual void get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *p_extensions) const override;
	virtual void get_recognized_extensions(List<String>* p_extensions) const;
  virtual void get_recognized_resources(List<String> *p_resources) const override;
	// virtual bool recognize(const Ref<Resource> &p_resource) const override;
};
}

#endif // SHADER_INCLUDE_H
