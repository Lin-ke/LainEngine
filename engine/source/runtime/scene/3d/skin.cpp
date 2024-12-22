#include "skin.h"

using namespace lain;

void Skin::set_bind_count(int p_size) {
	ERR_FAIL_COND(p_size < 0);
	binds.resize(p_size);
	binds_ptr = binds.ptrw();
	bind_count = p_size;
	emit_changed();
}

void Skin::add_bind(int p_bone, const Transform3D &p_pose) {
	uint32_t index = bind_count;
	set_bind_count(bind_count + 1);
	set_bind_bone(index, p_bone);
	set_bind_pose(index, p_pose);
}

void Skin::add_named_bind(const String &p_name, const Transform3D &p_pose) {
	uint32_t index = bind_count;
	set_bind_count(bind_count + 1);
	set_bind_name(index, p_name);
	set_bind_pose(index, p_pose);
}

void Skin::set_bind_name(int p_index, const StringName &p_name) {
	ERR_FAIL_INDEX(p_index, bind_count);
	bool notify_change = (binds_ptr[p_index].name != StringName()) != (p_name != StringName());
	binds_ptr[p_index].name = p_name;
	emit_changed();
	if (notify_change) {
		notify_property_list_changed();
	}
}

void Skin::set_bind_bone(int p_index, int p_bone) {
	ERR_FAIL_INDEX(p_index, bind_count);
	binds_ptr[p_index].bone = p_bone;
	emit_changed();
}

void Skin::set_bind_pose(int p_index, const Transform3D &p_pose) {
	ERR_FAIL_INDEX(p_index, bind_count);
	binds_ptr[p_index].pose = p_pose;
	emit_changed();
}

void Skin::clear_binds() {
	binds.clear();
	binds_ptr = nullptr;
	bind_count = 0;
	emit_changed();
}

void Skin::reset_state() {
	clear_binds();
}
