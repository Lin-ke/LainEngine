#pragma once
#ifndef TEST_DRAWTRIANGLE_H
#define TEST_DRAWTRIANGLE_H
#include "function/render/common/rendering_device.h"
namespace lain::test {
    void test_vulkan_driver() {
        using namespace lain;

        Ref<FileAccess> file = FileAccess::open("res://test1.glsl", FileAccess::READ);
        String shader_string = file->get_as_utf8_string();
        RenderingDevice* device = RenderingDevice::get_singleton();
        
        
    }
}
#endif