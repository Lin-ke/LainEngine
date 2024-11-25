#ifndef SKY_H
#define SKY_H

#include "core/os/thread.h"
#include "core/variant/binder_common.h"
#include "scene/resources/common/material.h"
#include "scene/resources/common/texture.h"
namespace lain{

class Sky : public Resource {
	LCLASS(Sky, Resource);

public:
	enum RadianceSize {
		RADIANCE_SIZE_32,
		RADIANCE_SIZE_64,
		RADIANCE_SIZE_128,
		RADIANCE_SIZE_256,
		RADIANCE_SIZE_512,
		RADIANCE_SIZE_1024,
		RADIANCE_SIZE_2048,
		RADIANCE_SIZE_MAX
	};

	enum ProcessMode {
		PROCESS_MODE_AUTOMATIC,
		PROCESS_MODE_QUALITY,
		PROCESS_MODE_INCREMENTAL,
		PROCESS_MODE_REALTIME
	};

private:
	RID sky;
	ProcessMode mode = PROCESS_MODE_AUTOMATIC;
	RadianceSize radiance_size = RADIANCE_SIZE_256;
	Ref<Material> sky_material;

protected:
	static void _bind_methods();

public:
	void set_radiance_size(RadianceSize p_size);
	RadianceSize get_radiance_size() const;

	void set_process_mode(ProcessMode p_mode);
	ProcessMode get_process_mode() const;

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

	virtual RID GetRID() const override;

	Sky();
	~Sky();
};

// VARIANT_ENUM_CAST(Sky::RadianceSize)
// VARIANT_ENUM_CAST(Sky::ProcessMode)
}

#endif // SKY_H
