#pragma once
#ifndef TEST_PROCESS_H
#define TEST_PROCESS_H
#include "test_marcos.h"
namespace lain {
	namespace test {
		void test_process() {
			Ref<PackedScene> s3 = ResourceLoader::load("3.tscn", "PackedScene");
			auto new_scene = s3->instantiate();
			TEQ(SceneTree::get_singleton()->get_root()->get_child_count(), 0);
			TEQ(SceneTree::get_singleton()->get_node_count(), 1);
			SceneTree::get_singleton()->get_root()->add_child(new_scene);


		}
	}
}
#endif
