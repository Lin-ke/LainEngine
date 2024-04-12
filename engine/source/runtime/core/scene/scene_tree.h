#pragma once
#ifndef SCENE_TREE_H
#define SCENE_TREE_H
#include "core/os/main_loop.h"
#include "core/os/thread_safe.h"
namespace lain {
	// 需要一个main viewport
	// viewport只保存渲染信息
	class GObject;
	class SceneTree : public MainLoop {
		_THREAD_SAFE_CLASS_
		friend class GObject;
		static SceneTree* singleton;
		GObject* current_scene = nullptr;
	public:
		struct Group {
			Vector<GObject*> nodes;
			bool changed = false;
		};
	private:
		HashMap<StringName, Group> group_map;
	public:
		SceneTree();
		~SceneTree();
		void tree_changed() {}

		Group* add_to_group(const StringName&, GObject*);

	};
}
#endif
