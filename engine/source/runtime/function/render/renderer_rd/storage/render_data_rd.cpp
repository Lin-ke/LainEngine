#include "render_data_rd.h"
using namespace lain;

Ref<RenderSceneBuffers> RenderDataRD::get_render_scene_buffers() const {
	return render_buffers;
}

RenderSceneData *RenderDataRD::get_render_scene_data() const {
	return scene_data;
}

RID RenderDataRD::get_environment() const {
	return environment;
}

RID RenderDataRD::get_camera_attributes() const {
	return camera_attributes;
}
