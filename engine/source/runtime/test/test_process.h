#pragma once
#ifndef TEST_PROCESS_H
#define TEST_PROCESS_H
#include "test_marcos.h"
#include "core/scene/component/component.h"
namespace lain {
	namespace test {
		

		class TestComponent : public Component{
			void _notification(int p_notification) {
				switch (p_notification) {
				case GObject::NOTIFICATION_PROCESS:
					L_PRINT("component tick");
				default:
					return;
				}
			}
		};

		void test_process() {
			List<Ref<Resource>> resources;

			ResourceCache::get_cached_resources(&resources);
			for (auto&& i : resources) {
				L_JSON(i->GetPath());
				L_JSON(i->GetName());
			}
			auto scenetree = SceneTree::get_singleton();
			Ref<PackedScene> s3 = ResourceLoader::load("3.tscn", "PackedScene");
			auto new_scene = s3->instantiate();
			TEQ(scenetree->get_root()->get_child_count(), 0);
			TEQ(scenetree->get_node_count(), 1);
			new_scene->tickdata.process_thread_group = TickObject::ProcessThreadGroup::PROCESS_THREAD_GROUP_SUB_THREAD;
			scenetree->get_root()->add_child(new_scene);
			new_scene->set_process(true);
			
			scenetree->process(0);

		}
	}
}
#endif
