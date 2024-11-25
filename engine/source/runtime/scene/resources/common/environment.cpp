#include "environment.h"
#include "function/render/rendering_system/rendering_system.h"
using namespace lain;
Environment::Environment() {
	environment = RS::get_singleton()->environment_create();

	set_camera_feed_id(bg_camera_feed_id);

	glow_levels.resize(7);
	glow_levels.write[0] = 0.0;
	glow_levels.write[1] = 0.0;
	glow_levels.write[2] = 1.0;
	glow_levels.write[3] = 0.0;
	glow_levels.write[4] = 1.0;
	glow_levels.write[5] = 0.0;
	glow_levels.write[6] = 0.0;

	_update_ambient_light();
	_update_tonemap();
	_update_ssr();
	_update_ssao();
	_update_ssil();
	_update_sdfgi();
	_update_glow();
	_update_fog();
	_update_adjustment();
	_update_volumetric_fog();
	_update_bg_energy();
	// notify_property_list_changed();
}


void Environment::_update_ambient_light() {
	RS::get_singleton()->environment_set_ambient_light(
			environment,
			ambient_color,
			RS::EnvironmentAmbientSource(ambient_source),
			ambient_energy,
			ambient_sky_contribution, RS::EnvironmentReflectionSource(reflection_source));
}


void Environment::_update_bg_energy() {
	if (GLOBAL_GET("rendering/lights_and_shadows/use_physical_light_units")) {
		RS::get_singleton()->environment_set_bg_energy(environment, bg_energy_multiplier, bg_intensity);
	} else {
		RS::get_singleton()->environment_set_bg_energy(environment, bg_energy_multiplier, 1.0);
	}
}

void Environment::set_camera_feed_id(int p_id) {
	bg_camera_feed_id = p_id;
// FIXME: Disabled during Vulkan refactoring, should be ported.
#if 0
	RS::get_singleton()->environment_set_camera_feed_id(environment, camera_feed_id);
#endif
}
