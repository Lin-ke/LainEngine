#pragma once
#ifndef TEST_SCENE_H
#define TEST_SCENE_H
#include "core/scene/object/gobject.h"
#include "core/scene/packed_scene.h"
#include "resource/io/resource_format_text.h"
#include "core/scene/component/component.h"
#include "core/scene/scene_tree.h"
namespace lain {
	namespace test {
		void draw_tree(GObject* root) {
			if (root == nullptr) {
				return;
			}
			List<GObject*> queue;
			queue.push_back(root);
			int level = 0;
			while (!queue.is_empty()) {
				L_PRINT("Layer:", level);
				int queue_size = queue.size();
				for (int i = 0; i < queue_size; ++i) {
					GObject* node = queue.front()->get();
					queue.pop_front();
					L_PRINT("name", CSTR(node->get_name().operator lain::String()));
					if(node->get_parent()!=nullptr)
						L_PRINT("father", CSTR(node->get_parent()->get_name().operator lain::String()));
					if (node->get_components().size() > 0) {
						L_JSON(node->get_components());
					}
					int childs_count = node->get_child_count();
					for (int j = 0; j < childs_count; j++) {
						queue.push_back(node->get_child(j));
					}
				}
				level++;
			}
		}
		void test_scene() {
			{
				GObject* scene = memnew(GObject);
				GObject* gobj1 = memnew(GObject);
				GObject* gobj2 = memnew(GObject);
				GObject* gobj1_1 = memnew(GObject);
				scene->set_name("TestScene");
				scene->add_component(memnew(Component));


				gobj1->set_name("hello");

				scene->add_child(gobj1);
				gobj1->set_owner(scene);

				gobj1_1->set_name("hello_child");
				gobj1->add_child(gobj1_1);
				gobj1_1->set_owner(scene);

				gobj2->set_name("world");
				scene->add_child(gobj2);
				gobj2->set_owner(scene);
				/*SceneTree::get_singleton()->get_root()->add_child(gobj1);
				SceneTree::get_singleton()->get_root()->add_child(gobj2);*/
				gobj1_1->add_component(memnew(Component));
				Ref<PackedScene> s;
				s.instantiate();
				s->pack(scene);
				ResourceSaver::save(s, "1.tscn");
				s->pack(scene);
				ResourceSaver::save(s, "2.tscn");
			}
			Ref<PackedScene> s;
			s.instantiate();
			Ref<PackedScene> s1 = ResourceLoader::load("1.tscn","PackedScene");
			GObject* newscene = s1->instantiate();
			L_JSON(newscene->get_components());
			Ref<PackedScene> s2 = ResourceLoader::load("2.tscn", "PackedScene");
			GObject* subscene = s2->instantiate();
			L_PRINT(subscene->data.instance_state.is_null(), CSTR(subscene->data.scene_file_path));
			List<Ref<Resource>> resources;
			ResourceCache::get_cached_resources(&resources);
			for (auto&& i : resources) {
				L_JSON(i->GetPath());
				L_JSON(i->GetName());
			}
			newscene->add_child(subscene);
			subscene->set_owner(newscene);
			// 查看是否reuse 可以
			//Ref<PackedScene> s3 = ResourceLoader::load("2.tscn", "PackedScene");
			
			s->pack(newscene);
			ResourceSaver::save(s, "3.tscn");
			Ref<PackedScene> s3 = ResourceLoader::load("3.tscn", "PackedScene");
			auto newscene_dup= s3->instantiate();
			// @TODO 清理todo ext resource
			draw_tree(newscene_dup);
			L_PRINT("---------");
			draw_tree(newscene);


		}

	}
}
#endif