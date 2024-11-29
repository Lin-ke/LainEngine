#ifndef WORLD_ENVIRONMENT_H
#define WORLD_ENVIRONMENT_H

#include "core/scene/object/gobject.h"
#include "scene/resources/common/camera_attributes.h"
#include "scene/resources/common/compositor.h"
#include "scene/resources/common/environment.h"
namespace lain{

class WorldEnvironment : public GObject {
	LCLASS(WorldEnvironment, GObject);

	Ref<Environment> environment;
	Ref<CameraAttributes> camera_attributes;
	Ref<Compositor> compositor;

	void _update_current_environment();
	void _update_current_camera_attributes();
	void _update_current_compositor();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_environment(const Ref<Environment> &p_environment);
	Ref<Environment> get_environment() const;

	void set_camera_attributes(const Ref<CameraAttributes> &p_camera_attributes);
	Ref<CameraAttributes> get_camera_attributes() const;

	void set_compositor(const Ref<Compositor> &p_compositor);
	Ref<Compositor> get_compositor() const;

	// PackedStringArray get_configuration_warnings() const override;

	WorldEnvironment();
};
}

#endif // WORLD_ENVIRONMENT_H
