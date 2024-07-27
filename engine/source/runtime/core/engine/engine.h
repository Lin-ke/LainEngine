#pragma once
#ifndef _ENGINE_H_

#define _ENGINE_H_// !_ENGINE_H_

#define ENGINE_GET(function_name, variable_name) \
L_INLINE auto function_name##() const { return variable_name; }

#define ENGINE_SET(function_name, variable_type, variable_name ) \
L_INLINE void function_name##(const variable_type##& p_##variable_name)  { variable_name = p_##variable_name; }


#include "base.h"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>
#include <unordered_set>
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
	bool m_use_validation_layers = false;
	bool m_abort_on_gpu_errors = false; // quit on gpu errors
	
public:
	ENGINE_GET(get_gpu_index,m_gpu_idx);
	ENGINE_GET(is_validation_layers_enabled, m_use_validation_layers);
	ENGINE_GET(is_abort_on_gpu_errors_enabled, m_abort_on_gpu_errors);
	ENGINE_GET(get_frame_ticks, m_frame_ticks);
	ENGINE_GET(is_generate_spirv_debug_info_enabled, m_generate_spirv_debug_info);
	ENGINE_GET(get_frame_fps, m_frame_fps);
	ENGINE_GET(get_fps, m_fps);
	ENGINE_GET(is_editor_hint, m_editor_hint);
	ENGINE_SET(set_editor_hint, bool, m_editor_hint);
	ENGINE_SET(set_frame_ticks, ui64 ,m_frame_ticks);


	Engine::Engine() {
		singleton = this;
	}
	L_INLINE static Engine* Engine::GetSingleton() {
		return singleton;
	}


};
}
#undef ENGINE_GET
#endif