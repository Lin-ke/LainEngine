#pragma once
#ifndef TEST_DRAWTRIANGLE_H
#define TEST_DRAWTRIANGLE_H
#include "function/render/rendering_device/rendering_device.h"
namespace lain::test {
    void test_vulkan_driver() {
        using namespace lain;

        Ref<FileAccess> file = FileAccess::open("res://test1.glsl", FileAccess::READ);
        L_PRINT(123);
        String shader_string = file->get_as_text();
        
        RenderingDevice* device = RenderingDevice::get_singleton();
        PackedByteArray shader = device->shader_compile_spirv_from_source(
            RenderingDevice::ShaderStage::SHADER_STAGE_VERTEX,
            shader_string,
            RenderingDevice::ShaderLanguage::SHADER_LANGUAGE_GLSL
        );
        
    }
}
#endif