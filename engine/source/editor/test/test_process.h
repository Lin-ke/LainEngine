#pragma once
#ifndef TEST_PROCESS_H
#define TEST_PROCESS_H
#include "test_marcos.h"
namespace lain {
	namespace test {
		void test_process() {
			auto scenetree = SceneTree::get_singleton();
			Ref<PackedScene> s3 = ResourceLoader::load("3.tscn", "PackedScene");
			auto new_scene = s3->instantiate();
			TEQ(scenetree->get_root()->get_child_count(), 0);
			TEQ(scenetree->get_node_count(), 1);
			scenetree->get_root()->add_child(new_scene);
			new_scene->set_process(true);
			
			scenetree->process(0);

		}
	}
}
#endif
