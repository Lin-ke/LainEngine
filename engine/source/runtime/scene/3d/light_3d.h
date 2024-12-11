#ifndef LIGHT_3D_H
#define LIGHT_3D_H

#include "scene/3d/visual_instance_3d.h"
namespace lain{
class Light3D : public VisualInstance3D {
  LCLASS(Light3D, VisualInstance3D);
  public:
  	enum Param {
		PARAM_ENERGY = RS::LIGHT_PARAM_ENERGY,
		PARAM_INDIRECT_ENERGY = RS::LIGHT_PARAM_INDIRECT_ENERGY,
		PARAM_VOLUMETRIC_FOG_ENERGY = RS::LIGHT_PARAM_VOLUMETRIC_FOG_ENERGY,
		PARAM_SPECULAR = RS::LIGHT_PARAM_SPECULAR,
		PARAM_RANGE = RS::LIGHT_PARAM_RANGE,
		PARAM_SIZE = RS::LIGHT_PARAM_SIZE,
		PARAM_ATTENUATION = RS::LIGHT_PARAM_ATTENUATION,
		PARAM_SPOT_ANGLE = RS::LIGHT_PARAM_SPOT_ANGLE,
		PARAM_SPOT_ATTENUATION = RS::LIGHT_PARAM_SPOT_ATTENUATION,
		PARAM_SHADOW_MAX_DISTANCE = RS::LIGHT_PARAM_SHADOW_MAX_DISTANCE,
		PARAM_SHADOW_SPLIT_1_OFFSET = RS::LIGHT_PARAM_SHADOW_SPLIT_1_OFFSET,
		PARAM_SHADOW_SPLIT_2_OFFSET = RS::LIGHT_PARAM_SHADOW_SPLIT_2_OFFSET,
		PARAM_SHADOW_SPLIT_3_OFFSET = RS::LIGHT_PARAM_SHADOW_SPLIT_3_OFFSET,
		PARAM_SHADOW_FADE_START = RS::LIGHT_PARAM_SHADOW_FADE_START,
		PARAM_SHADOW_NORMAL_BIAS = RS::LIGHT_PARAM_SHADOW_NORMAL_BIAS,
		PARAM_SHADOW_BIAS = RS::LIGHT_PARAM_SHADOW_BIAS,
		PARAM_SHADOW_PANCAKE_SIZE = RS::LIGHT_PARAM_SHADOW_PANCAKE_SIZE,
		PARAM_SHADOW_OPACITY = RS::LIGHT_PARAM_SHADOW_OPACITY,
		PARAM_SHADOW_BLUR = RS::LIGHT_PARAM_SHADOW_BLUR,
		PARAM_TRANSMITTANCE_BIAS = RS::LIGHT_PARAM_TRANSMITTANCE_BIAS,
		PARAM_INTENSITY = RS::LIGHT_PARAM_INTENSITY,
		PARAM_MAX = RS::LIGHT_PARAM_MAX
	};
  	enum BakeMode {
		BAKE_DISABLED,
		BAKE_STATIC,
		BAKE_DYNAMIC,
	};

private:
	Color color;
	real_t param[PARAM_MAX] = {};
	bool shadow = false;
	bool negative = false;
	bool reverse_cull = false;
	uint32_t cull_mask = 0;
	bool distance_fade_enabled = false;
	real_t distance_fade_begin = 40.0;
	real_t distance_fade_shadow = 50.0;
	real_t distance_fade_length = 10.0;
	RS::LightType type = RS::LIGHT_DIRECTIONAL;
	bool editor_only = false;
	void _update_visibility();
	BakeMode bake_mode = BAKE_DYNAMIC;
	Ref<Texture2D> projector;
	Color correlated_color = Color(1.0, 1.0, 1.0);
	float temperature = 6500.0;

protected:
	RID light;

	static void _bind_methods();
	void _notification(int p_what);
	void _validate_property(PropertyInfo &p_property) const;

	Light3D(RS::LightType p_type);

public:
	RS::LightType get_light_type() const { return type; }
	void set_param(Param p_param, real_t p_value);
	real_t get_param(Param p_param) const;
  
  	void set_shadow(bool p_enable);
	bool has_shadow() const;

  
	void set_color(const Color &p_color);
	Color get_color() const;

	~Light3D();

};
VARIANT_ENUM_CAST(Light3D::Param);
VARIANT_ENUM_CAST(Light3D::BakeMode);


class DirectionalLight3D : public Light3D {
  LCLASS(DirectionalLight3D, Light3D);
  public:
  	enum ShadowMode {
		SHADOW_ORTHOGONAL,
		SHADOW_PARALLEL_2_SPLITS,
		SHADOW_PARALLEL_4_SPLITS,
	};
  enum SkyMode {
		SKY_MODE_LIGHT_AND_SKY,
		SKY_MODE_LIGHT_ONLY,
		SKY_MODE_SKY_ONLY,
	};
  private:
	ShadowMode shadow_mode;
  bool blend_splits;
  SkyMode sky_mode;
  protected:
  static void _bind_methods();
  public:
	void set_shadow_mode(ShadowMode p_mode);
	ShadowMode get_shadow_mode() const;
  DirectionalLight3D();

  
	void set_sky_mode(SkyMode p_mode);
	SkyMode get_sky_mode() const;

};

VARIANT_ENUM_CAST(DirectionalLight3D::ShadowMode)

class OmniLight3D : public Light3D{
  LCLASS(OmniLight3D, Light3D);
  public:
  	// omni light
	enum ShadowMode {
		SHADOW_DUAL_PARABOLOID,
		SHADOW_CUBE,
	};
  
private:
	ShadowMode shadow_mode;
  protected:
	static void _bind_methods();

public:
	void set_shadow_mode(ShadowMode p_mode);
	ShadowMode get_shadow_mode() const;

	// PackedStringArray get_configuration_warnings() const override;

	OmniLight3D();
};
VARIANT_ENUM_CAST(OmniLight3D::ShadowMode)

}
#endif