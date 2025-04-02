#pragma once
#ifndef _ENGINE_H_

#define _ENGINE_H_  // !_ENGINE_H_

#define ENGINE_GET(function_name, variable_name) \
  L_INLINE auto function_name##() const {        \
    return variable_name;                        \
  }

#define ENGINE_SET(function_name, variable_type, variable_name)             \
  L_INLINE void function_name##(const variable_type##& p_##variable_name) { \
    variable_name = p_##variable_name;                                      \
  }

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>
#include <unordered_set>
#include "base.h"
#include "core/string/ustring.h"
namespace lain {

class Engine {
 private:
  static Engine* singleton;

 public:
  ui64 m_frame_ticks = 0;
  ui64 m_frame_fps = 0;
  double m_fps = 1;
  bool m_editor_hint = false;
  /// --- configs
  int32_t m_gpu_idx = 0;
  bool m_generate_spirv_debug_info = false;
  bool m_use_validation_layers = true;
  bool m_abort_on_gpu_errors = false;  // quit on gpu errors

  static constexpr int SERVER_SYNC_FRAME_COUNT_WARNING = 5;
  bool m_frame_server_synced = false;  // 这是干什么？
  int m_server_syncs = 0;
  int m_frames_drawn = 0;
  
	uint64_t _physics_frames = 0;
	uint64_t _process_frames = 0;

  // 这个设置在 RendererCompositor里，但是会导致shader Parser 依赖renderer
  bool m_xr_enabled = false;
  float m_time_scale = 1.0;

// 与时间和同步相关的参数
	double physics_jitter_fix = 0.5;
  int ips = 60;
  // shader cache
  String shader_cache_path ;

 public:
  ENGINE_GET(get_gpu_index, m_gpu_idx);
  ENGINE_GET(is_validation_layers_enabled, m_use_validation_layers);
  ENGINE_GET(is_abort_on_gpu_errors_enabled, m_abort_on_gpu_errors);
  ENGINE_GET(get_frame_ticks, m_frame_ticks);
  ENGINE_GET(get_time_scale, m_time_scale);
  ENGINE_GET(is_generate_spirv_debug_info_enabled, m_generate_spirv_debug_info);
  ENGINE_GET(get_frame_fps, m_frame_fps);
  ENGINE_GET(get_fps, m_fps);
  ENGINE_GET(get_physics_jitter_fix, physics_jitter_fix);
  ENGINE_GET(is_editor_hint, m_editor_hint);
  ENGINE_GET(get_frames_drawn, m_frames_drawn);
  ENGINE_SET(set_editor_hint, bool, m_editor_hint);
  ENGINE_SET(set_frame_ticks, ui64, m_frame_ticks);
  ENGINE_GET(is_xr_enabled, m_xr_enabled);
  ENGINE_GET(get_physics_ticks_per_second, ips);
  ENGINE_GET(get_physics_frames, _physics_frames);
  ENGINE_GET(get_process_frames, _process_frames);
  ENGINE_SET(set_shader_cache_path,String, shader_cache_path);
  ENGINE_GET(get_shader_cache_path, shader_cache_path);

  Engine::Engine() { singleton = this; }
  L_INLINE static Engine* Engine::GetSingleton() { return singleton; }
  L_INLINE void increment_frames_drawn() { m_frames_drawn++; }
  bool notify_frame_server_synced() {
    m_frame_server_synced = true;
    return m_server_syncs > SERVER_SYNC_FRAME_COUNT_WARNING;
  }
};
}  // namespace lain
#undef ENGINE_GET
#endif