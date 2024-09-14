#pragma once
#ifndef TEST_DRAWTRIANGLE_H
#define TEST_DRAWTRIANGLE_H
#include "function/render/rendering_device/rendering_device.h"
#include "core/io/json11.h"

namespace lain::test {
void test_draw_triangle() {
  using namespace lain;
  Json v = Json::object{{"a", 1}, {"b", 2}};
  L_PRINT(v.dump());
  Ref<FileAccess> file1 = FileAccess::open("res://1.tscn", FileAccess::READ);
  String text = file1->get_as_text();
  String err_text = "";
  Json json = Json::parse(text, err_text);
  L_PRINT(err_text);
  L_PRINT(json.dump());
  Ref<FileAccess> file2= FileAccess::open("res://test1.glsl", FileAccess::READ);
  String vs_shader_string = file2->get_as_text();

  RenderingDevice* device = RenderingDevice::get_singleton();
  PackedByteArray vs_shader =
      device->shader_compile_spirv_from_source(RenderingDevice::ShaderStage::SHADER_STAGE_VERTEX, vs_shader_string, RenderingDevice::ShaderLanguage::SHADER_LANGUAGE_GLSL);
  Ref<FileAccess> file = FileAccess::open("res://test1_fs.glsl", FileAccess::READ);
  String fs_shader_string = file->get_as_text();
  PackedByteArray fs_shader =
      device->shader_compile_spirv_from_source(RenderingDevice::ShaderStage::SHADER_STAGE_FRAGMENT, fs_shader_string, RenderingDevice::ShaderLanguage::SHADER_LANGUAGE_GLSL);
  
  RD::ShaderStageSPIRVData vs_data;
  vs_data.shader_stage = RD::ShaderStage::SHADER_STAGE_VERTEX;
  vs_data.spirv = vs_shader;

  RD::ShaderStageSPIRVData fs_data;
  fs_data.shader_stage = RD::ShaderStage::SHADER_STAGE_FRAGMENT;
  fs_data.spirv = fs_shader;

  RD::VertexAttribute attr;  // 应该有一个create vertex format for shader的函数 直接反射过来
  attr.format = RD::DataFormat::DATA_FORMAT_R32G32B32_SFLOAT;
  attr.frequency = RD::VertexFrequency::VERTEX_FREQUENCY_VERTEX;
  RD::VertexFormatID vertex_format_id = device->vertex_format_create({attr});
  auto points = Vector<float>{1, 1, 0, -1, 1, 0, 0, -1, 0}.to_byte_array();

  RID vertex_buffer_id = device->vertex_buffer_create(points.size(), points);
  RID vertex_array_id  = device->vertex_array_create(3, vertex_format_id, {vertex_buffer_id});

  RD::PipelineColorBlendState blend;
  blend.attachments = {RD::PipelineColorBlendState::Attachment()};

  auto tex_format = RD::TextureFormat();
  auto tex_view = RD::TextureView();
  Size2i window_size = WindowSystem::GetSingleton()->window_get_size(WindowSystem::MAIN_WINDOW_ID);
  L_PRINT(window_size);
  tex_format.height = window_size.y;
  tex_format.width = window_size.x;
  tex_format.format = RD::DataFormat::DATA_FORMAT_R32G32B32A32_SFLOAT;
  tex_format.usage_bits = RD::TextureUsageBits::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TextureUsageBits::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
  auto framebuf_texture = device->texture_create(tex_format, tex_view);
  RD::AttachmentFormat format = RD::AttachmentFormat(tex_format);  // 兼容
  auto framebuf_format = device->framebuffer_format_create({format});
  auto framebuf = device->framebuffer_create({framebuf_texture}, framebuf_format);
  // 读到create multi pass.
  auto Shader_rid = device->shader_create_from_spirv({vs_data, fs_data}, "vertex_test");
  auto pipeline = device->render_pipeline_create(
    Shader_rid, framebuf_format, vertex_format_id, RD::RenderPrimitive::RENDER_PRIMITIVE_TRIANGLES,
    RD::PipelineRasterizationState(), RD::PipelineMultisampleState(), 
    RD::PipelineDepthStencilState(), RD::PipelineColorBlendState::create_blend()

  );
  auto clear_color_values= PackedColorArray({Color(1,1,1,1)});
  // auto draw_list = device->draw_list_begin(
  //   framebuf, RD::ColorInitialAction(), RD::ColorFinalAction(), RD::InitialAction::INITIAL_ACTION_CLEAR, RD::FinalAction::FINAL_ACTION_STORE,
  //   clear_color_values
  // );
  Error err = device->screen_prepare_for_drawing(WindowSystem::MAIN_WINDOW_ID);

  auto draw_list_screen = device->draw_list_begin_for_screen();
  
  device->draw_list_bind_render_pipeline(draw_list_screen, pipeline);
  device->draw_list_bind_vertex_array(draw_list_screen,vertex_array_id);
  device->draw_list_draw(draw_list_screen, false, 3, 0);
  device->draw_list_end();

}
}  // namespace lain::test
#endif