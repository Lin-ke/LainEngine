#ifndef CAMERA_ATTRIBUTES_H
#define CAMERA_ATTRIBUTES_H

#include "core/io/resource.h"
#include "core/io/rid.h"
namespace lain{

class CameraAttributes : public Resource {
  LCLASS(CameraAttributes, Resource);
  private:
	RID camera_attributes;
  public:
  virtual RID GetRID() const { return camera_attributes; }
  CameraAttributes();
  ~CameraAttributes();
	virtual void _update_auto_exposure() {};
	virtual float calculate_exposure_normalization() const { return 1.0; }

};

class CameraAttributesPhysical : public CameraAttributes {
	LCLASS(CameraAttributesPhysical, CameraAttributes);

private:
	// Exposure
	float exposure_aperture = 16.0; // In f-stops;
	float exposure_shutter_speed = 100.0; // In 1 / seconds;

	// Camera properties.
	float frustum_focal_length = 35.0; // In millimeters.
	float frustum_focus_distance = 10.0; // In Meters.
	real_t frustum_near = 0.05;
	real_t frustum_far = 4000.0;
	real_t frustum_fov = 75.0;
	void _update_frustum();

	virtual void _update_auto_exposure() override;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &property) const;

public:
	void set_aperture(float p_aperture);
	float get_aperture() const;

	void set_shutter_speed(float p_shutter_speed);
	float get_shutter_speed() const;

	void set_focal_length(float p_focal_length);
	float get_focal_length() const;

	void set_focus_distance(float p_focus_distance);
	float get_focus_distance() const;

	void set_near(real_t p_near);
	real_t get_near() const;

	void set_far(real_t p_far);
	real_t get_far() const;

	real_t get_fov() const;

	void set_auto_exposure_min_exposure_value(float p_min);
	float get_auto_exposure_min_exposure_value() const;
	void set_auto_exposure_max_exposure_value(float p_max);
	float get_auto_exposure_max_exposure_value() const;

	virtual float calculate_exposure_normalization() const override;

	CameraAttributesPhysical();
	~CameraAttributesPhysical();
};
}


#endif 