#pragma once
#ifndef SCENE_TREE_H
#define SCENE_TREE_H
#include "core/os/main_loop.h"
#include "core/os/thread_safe.h"
#include "core/templates/local_vector.h"
#include "core/templates/hash_set.h"
namespace lain {
	// 需要一个main viewport
	// viewport只保存渲染信息
	class GObject;
	class Component;
	class TickObject;
	class SceneTree : public MainLoop {
		_THREAD_SAFE_CLASS_


		friend class TickObject;
		friend class GObject;
		friend class Component;
		static SceneTree* singleton;
		GObject* current_scene;
		GObject* root; // window
		// state
		int root_lock = 0;
		bool paused = false;
		bool _quit = false;
		bool processing = false;
		double process_time = 0.0;

		// settings
		bool node_threading_disabled = false;

	public:
		struct Group {
			Vector<GObject*> nodes;
			bool changed = false;
		};
		enum {
		NOTIFICATION_TRANSFORM_CHANGED = 2000
	};
	private:
		uint64_t process_last_pass = 1;
		struct ProcessGroup {
			//CallQueue call_queue;
			Vector<TickObject*> nodes;
			Vector<TickObject*> physics_nodes;
			bool node_order_dirty = true;
			bool physics_node_order_dirty = true;
			bool removed = false;
			TickObject* owner = nullptr;
			uint64_t last_pass = 0;
		};
		struct ProcessGroupSort {
			_FORCE_INLINE_ bool operator()(const ProcessGroup* p_left, const ProcessGroup* p_right) const;
		};
		// process by group
		PagedAllocator<ProcessGroup, true> group_allocator; // Allocate groups on pages, to enhance cache usage.
		
		LocalVector<ProcessGroup*> process_groups;
		HashMap<StringName, Group> group_map;
		bool process_groups_dirty = true;
		LocalVector<ProcessGroup*> local_process_group_cache; // Used when processing to group what needs to
		ProcessGroup default_process_group; // 有p_owner就加到那个，否则加到这个


		// Safety for when a node is deleted while a group is being called.
		int nodes_removed_on_group_call_lock = 0;
		HashSet<TickObject*> nodes_removed_on_group_call; // Skip erased nodes.
		int nodes_in_tree_count = 0;


		void _process(bool);
		void _process_group(ProcessGroup* p_group, bool p_physics);
		void _process_groups_thread(uint32_t p_index, bool p_physics);

		void _remove_process_group(TickObject* p_node);
		void _add_process_group(TickObject* p_node);
		void _remove_node_from_process_group(TickObject* p_node, TickObject* p_owner);
		void _add_node_to_process_group(TickObject* p_node, TickObject* p_owner);


	public:
		SceneTree();
		~SceneTree();
		L_INLINE static SceneTree* get_singleton() { return singleton; }

		virtual void initialize() override;
		virtual void finalize() override;

		void tree_changed() {}
		L_INLINE GObject* get_root() const { return root; }
		Group* add_to_group(const StringName&, GObject*);
		virtual bool process(double p_time) override;
		L_INLINE int get_node_count() {
			return nodes_in_tree_count;
		}
		/// signal method
		void node_removed(GObject* node);

		// state
		bool is_paused() const { return paused; }
	};
}
#endif
