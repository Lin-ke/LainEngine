#ifndef SKY_MATERIAL_H
#define SKY_MATERIAL_H

#include "core/io/rid.h"
#include "scene/resources/common/material.h"

namespace lain{

class PanoramaSkyMaterial : public Material {
	LCLASS(PanoramaSkyMaterial, Material);

private:
	Ref<Texture2D> panorama;
	float energy_multiplier = 1.0f;

	static Mutex shader_mutex;
	static RID shader_cache[2];
	static void _update_shader();
	mutable bool shader_set = false;

	bool filter = true;

protected:
	static void _bind_methods();

public:
	void set_panorama(const Ref<Texture2D> &p_panorama);
	Ref<Texture2D> get_panorama() const;

	void set_filtering_enabled(bool p_enabled);
	bool is_filtering_enabled() const;

	void set_energy_multiplier(float p_multiplier);
	float get_energy_multiplier() const;

	virtual Shader::Mode get_shader_mode() const override;
	virtual RID get_shader_rid() const override;
	virtual RID GetRID() const override;

	static void cleanup_shader();

	PanoramaSkyMaterial();
	~PanoramaSkyMaterial();
};



class PhysicalSkyMaterial : public Material {
	LCLASS(PhysicalSkyMaterial, Material);

private:
	static Mutex shader_mutex;
	static RID shader_cache[2];

	float rayleigh = 0.0f;
	Color rayleigh_color;
	float mie = 0.0f;
	float mie_eccentricity = 0.0f;
	Color mie_color;
	float turbidity = 0.0f;
	float sun_disk_scale = 0.0f;
	Color ground_color;
	float energy_multiplier = 1.0f;
	bool use_debanding = true;
	Ref<Texture2D> night_sky;
	static void _update_shader();
	mutable bool shader_set = false;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &property) const;

public:
	void set_rayleigh_coefficient(float p_rayleigh);
	float get_rayleigh_coefficient() const;

	void set_rayleigh_color(Color p_rayleigh_color);
	Color get_rayleigh_color() const;

	void set_turbidity(float p_turbidity);
	float get_turbidity() const;

	void set_mie_coefficient(float p_mie);
	float get_mie_coefficient() const;

	void set_mie_eccentricity(float p_eccentricity);
	float get_mie_eccentricity() const;

	void set_mie_color(Color p_mie_color);
	Color get_mie_color() const;

	void set_sun_disk_scale(float p_sun_disk_scale);
	float get_sun_disk_scale() const;

	void set_ground_color(Color p_ground_color);
	Color get_ground_color() const;

	void set_energy_multiplier(float p_multiplier);
	float get_energy_multiplier() const;

	void set_exposure_value(float p_exposure);
	float get_exposure_value() const;

	void set_use_debanding(bool p_use_debanding);
	bool get_use_debanding() const;

	void set_night_sky(const Ref<Texture2D> &p_night_sky);
	Ref<Texture2D> get_night_sky() const;

	virtual Shader::Mode get_shader_mode() const override;
	virtual RID get_shader_rid() const override;

	static void cleanup_shader();
	virtual RID GetRID() const override;

	PhysicalSkyMaterial();
	~PhysicalSkyMaterial();
};
}

#endif