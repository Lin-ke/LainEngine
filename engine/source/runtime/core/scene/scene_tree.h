#pragma once
#ifndef SCENE_TREE_H
#define SCENE_TREE_H
#include "core/os/main_loop.h"
#include "core/scene/object/gobject.h"
namespace lain {
	// 需要一个main viewport
	// viewport只保存渲染信息
	class SceneTree : public MainLoop {
		static SceneTree* singleton;
		
		GObject* current_scene = nullptr;
		SceneTree();
		~SceneTree();
	};
}
#endif
