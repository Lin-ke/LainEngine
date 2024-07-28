#pragma once
#ifndef TEST_VK_H
#define TEST_VK_H
#include "function/render/common/rendering_device.h"
namespace lain::test {
	void test_vulkan_driver() {
		using namespace lain::graphics;

		Ref<FileAccess> file = FileAccess::open("res://test1.glsl", FileAccess::READ);
		String str = file->get_as_utf8_string();
		RenderingDeviceDriver* driver = RenderingDevice::get_singleton()->get_driver();
		auto a = driver->buffer_create(100, RDD::BUFFER_USAGE_UNIFORM_BIT, RDD::MEMORY_ALLOCATION_TYPE_CPU);
		// auto b = driver->texture_create({}, {} );
		RenderingDevice::get_singleton()->texture_create(
			{}, {}
		);
		auto index = driver->command_queue_family_get(
			RDD::CommandQueueFamilyBits::COMMAND_QUEUE_FAMILY_GRAPHICS_BIT
		);
		auto c = driver->command_pool_create(
			index, RDD::CommandBufferType::COMMAND_BUFFER_TYPE_PRIMARY
		)	;
		auto d  = RenderingDevice::get_singleton()->framebuffer_format_create_empty();
		auto e = RenderingDevice::get_singleton()->framebuffer_create_empty({720,480}, RDD::TextureSamples::TEXTURE_SAMPLES_1, d);
		L_PRINT(d, e.get_id());		
		Vector<RenderingDevice::AttachmentFormat> formats = {
			{}, {}, {}
		};
		auto f = RenderingDevice::get_singleton()->framebuffer_format_create(formats);
		auto g = RenderingDevice::get_singleton()->framebuffer_create_empty({720, 480}, RDD::TextureSamples::TEXTURE_SAMPLES_1, d);
		L_PRINT(f, g.get_id());
		
	}

}
#endif