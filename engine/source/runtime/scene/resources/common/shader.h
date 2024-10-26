#pragma once
#ifndef __SHADER__H__
#define __SHADER__H__
#include "core/io/resource.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "scene/resources/common/texture.h"
#include "shader_include.h"
namespace lain {
// Shader -> Shader compiler -> Uber shader -> Compile to SPRIV

class Shader : public Resource {
  LCLASS(Shader, Resource);
  OBJ_SAVE_TYPE(Shader);

 public:
  enum Mode { MODE_SPATIAL, MODE_CANVAS_ITEM, MODE_PARTICLES, MODE_SKY, MODE_FOG, MODE_MAX };
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
  Mode mode = MODE_SPATIAL;  // 我感觉这不是一个好设计
  HashSet<Ref<ShaderInclude>> include_dependencies;
  String code;
  String include_path;

  virtual Mode get_mode() const;

  virtual void set_path(const String& p_path, bool p_take_over = false) override;
  L_INLINE void set_include_path(const String& p_path) { include_path = p_path; }

  void set_code(const String& p_code);
  String get_code() const;
  virtual void _update_shader() const{} //used for visual shader

  void get_shader_uniform_list(List<PropertyInfo>* p_params, bool p_get_groups = false) const;

  void set_default_texture_parameter(const StringName& p_name, const Ref<Texture2D>& p_texture, int p_index = 0);
  Ref<Texture2D> get_default_texture_parameter(const StringName& p_name, int p_index = 0) const;
  void get_default_texture_parameter_list(List<StringName>* r_textures) const;

  virtual bool is_text_shader() const { return true; }

  virtual RID GetRID() const override { 
	_update_shader();
	return shader; }

  Shader();
  ~Shader();
};

}  // namespace lain
#endif