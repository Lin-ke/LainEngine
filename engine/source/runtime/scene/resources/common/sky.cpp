// #include "sky.h"

// #include "core/io/image_loader.h"
// #include "function/render/rendering_system/rendering_system.h"
// using namespace lain;
// void Sky::set_radiance_size(RadianceSize p_size) {
// 	ERR_FAIL_INDEX(p_size, RADIANCE_SIZE_MAX);

// 	radiance_size = p_size;
// 	static const int size[RADIANCE_SIZE_MAX] = {
// 		32, 64, 128, 256, 512, 1024, 2048
// 	};
// 	RS::get_singleton()->sky_set_radiance_size(sky, size[radiance_size]);
// }

// Sky::RadianceSize Sky::get_radiance_size() const {
// 	return radiance_size;
// }

// void Sky::set_process_mode(ProcessMode p_mode) {
// 	mode = p_mode;
// 	RS::get_singleton()->sky_set_mode(sky, RS::SkyMode(mode));
// }

// Sky::ProcessMode Sky::get_process_mode() const {
// 	return mode;
// }

// void Sky::set_material(const Ref<Material> &p_material) {
// 	sky_material = p_material;
// 	RID material_rid;
// 	if (sky_material.is_valid()) {
// 		material_rid = sky_material->get_rid();
// 	}
// 	RS::get_singleton()->sky_set_material(sky, material_rid);
// }

// Ref<Material> Sky::get_material() const {
// 	return sky_material;
// }

// RID Sky::get_rid() const {
// 	return sky;
// }