#pragma once
#ifndef TEST_DRAWTRIANGLE_H
#define TEST_DRAWTRIANGLE_H
#include "function/render/rendering_device/rendering_device.h"
namespace lain::test {
void test_draw_triangle() {
  using namespace lain;

  Ref<FileAccess> file = FileAccess::open("res://test1.glsl", FileAccess::READ);
  String shader_string = file->get_as_text();

  RenderingDevice* device = RenderingDevice::get_singleton();
  PackedByteArray shader =
      device->shader_compile_spirv_from_source(RenderingDevice::ShaderStage::SHADER_STAGE_VERTEX, shader_string, RenderingDevice::ShaderLanguage::SHADER_LANGUAGE_GLSL);
  RD::ShaderStageSPIRVData data;
  data.shader_stage = RD::ShaderStage::SHADER_STAGE_VERTEX;
  data.spirv = shader;

  device->shader_compile_binary_from_spirv({data}, "vertex_test");
  RD::VertexAttribute attr;  // 应该有一个create vertex format for shader的函数 直接反射过来
  attr.format = RD::DataFormat::DATA_FORMAT_R32G32B32_SFLOAT;
  attr.frequency = RD::VertexFrequency::VERTEX_FREQUENCY_VERTEX;
  RD::VertexFormatID vertex_format_id = device->vertex_format_create({attr});
  auto points = Vector<float>{1, 1, 0, -1, 1, 0, 0, -1, 0}.to_byte_array();

  RID vertex_buffer_id = device->vertex_buffer_create(points.size(), points);
  RD::PipelineColorBlendState blend;
  blend.attachments = {RD::PipelineColorBlendState::Attachment()};

  auto tex_format = RD::TextureFormat();
  auto tex_view = RD::TextureView();
  tex_format.height = 800;
  tex_format.width = 600;
  tex_format.format = RD::DataFormat::DATA_FORMAT_R32G32B32A32_SFLOAT;
  tex_format.usage_bits = RD::TextureUsageBits::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TextureUsageBits::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
  auto framebuf_texture = device->texture_create(tex_format, tex_view);
  RD::AttachmentFormat format = RD::AttachmentFormat(tex_format);  // 兼容
  auto framebuf_format = device->framebuffer_format_create({format});
  device->framebuffer_create({framebuf_texture}, framebuf_format);
  // 读到create multi pass.
  auto pipeline = device->render_pipeline_create(
    RID(), framebuf_format, vertex_format_id
  );
  

}
}  // namespace lain::test
#endif