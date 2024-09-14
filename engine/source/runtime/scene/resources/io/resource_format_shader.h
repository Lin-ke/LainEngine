#pragma once
#ifndef RESOURCE_FORMAT_SHADER_H
#define RESOURCE_FORMAT_SHADER_H
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"

namespace lain{

class ResourceFormatLoaderShader : public ResourceFormatLoader {
public:
	virtual Ref<Resource> load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_use_sub_threads = false, float *r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
    virtual void get_recognized_extensions(List<String> *p_extensions) const override;
    virtual void get_recognized_resources(List<String> *p_resource_class) const override;
};

class ResourceFormatSaverShader : public ResourceFormatSaver {
public:
	virtual Error save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags = 0) override;
	virtual void get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *p_extensions) const override;
    virtual void get_recognized_extensions(List<String> *p_extensions) const override;
    virtual void get_recognized_resources(List<String> *p_resource_class) const override;
};

class ResourceFormatLoaderRawShader : public ResourceFormatLoader {
    virtual void get_recognized_extensions(List<String> *p_extensions) const override{
        p_extensions->push_back("glsl");
    }
    virtual void get_recognized_resources(List<String> *p_resource_class) const override{
        p_resource_class->push_back("ShaderFile");
    }



};
}
#endif // RESOURCE_FORMAT_SHADER_H
