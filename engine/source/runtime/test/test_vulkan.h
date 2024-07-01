#pragma once
#ifndef TEST_VK_H
#define TEST_VK_H
#include "function/render/common/rendering_device.h"
namespace lain::test {
	void test_vulkan_driver() {
		using namespace lain::graphics;

		Ref<FileAccess> file = FileAccess::open("res://test1.glsl", FileAccess::READ);
		String f = file->get_as_utf8_string();
		RenderingDeviceDriver* driver = RenderingDevice::get_singleton()->get_driver();
		auto a = driver->buffer_create(100, RDD::BUFFER_USAGE_UNIFORM_BIT, RDD::MEMORY_ALLOCATION_TYPE_CPU);
		auto b = driver->texture_create({}, {} );
		auto index = driver->command_queue_family_get(
			RDD::CommandQueueFamilyBits::COMMAND_QUEUE_FAMILY_GRAPHICS_BIT
		);
		auto c = driver->command_pool_create(
			index, RDD::CommandBufferType::COMMAND_BUFFER_TYPE_PRIMARY
		)	;

	}

}
#endif