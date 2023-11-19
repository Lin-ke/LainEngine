#pragma once
#ifndef _ENGINE_H_

#define _ENGINE_H_// !_ENGINE_H_
#include "base.h"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>
#include <unordered_set>

class Engine {
public:

	static Engine* singleton;
	u64 m_frame_ticks = 0;
	u64 m_frame_fps = 0;
	double m_fps = 1;


	Engine::Engine() {
		singleton = this;
	}
	L_INLINE static Engine* Engine::GetSingleton() {
		return singleton;
	}


};
#endif