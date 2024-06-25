#pragma once
#ifndef TEST_VK_H
#define TEST_VK_H
#include "function/render/common/rendering_device.h"
namespace lain::test {
	void test_shader_compile() {
		using namespace lain::graphics;
		Ref<FileAccess> file = FileAccess::open("res://test1.glsl", FileAccess::READ);
		String f = file->get_as_utf8_string();
		RenderingDeviceDriver* driver = RenderingDevice::get_singleton()->get_driver();
		driver->shader_compile_binary_from_spirv(f);
	}
}
#endif