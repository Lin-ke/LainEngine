#pragma once
#ifndef SCENE_TREE_H
#define SCENE_TREE_H
#include "core/os/main_loop.h"
#include "core/scene/object/gobject.h"
namespace lain {
	// ��Ҫһ��main viewport
	// viewportֻ������Ⱦ��Ϣ
	class SceneTree : public MainLoop {
		static SceneTree* singleton;
		
		GObject* current_scene = nullptr;
		SceneTree();
		~SceneTree();
	};
}
#endif
