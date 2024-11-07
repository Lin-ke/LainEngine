#ifndef RENDER_DATA_H
#define RENDER_DATA_H

#include "core/object/object.h"
#include "render_scene_buffers_api.h"
#include "render_scene_data_api.h"
namespace lain {
class RenderData : public Object {
	LCLASS(RenderData, Object);

protected:
	// static void _bind_methods();

public:
	virtual Ref<RenderSceneBuffers> get_render_scene_buffers() const = 0;
	virtual RenderSceneData *get_render_scene_data() const = 0;

	virtual RID get_environment() const = 0;
	virtual RID get_camera_attributes() const = 0;
};
}
#endif