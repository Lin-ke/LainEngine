#include "test_scene.h"
#include "core/scene/packed_scene.h"
#include "resource/io/resource_format_text.h"
#include "core/scene/component/component.h"
#include "core/scene/scene_tree.h"
#include "core/meta/serializer/serializer.h"
#include "../_generated/serializer/all_serializer.h"

namespace lain {
	namespace test {
		void draw_tree(GObject* root)
		{
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
					L_PRINT("name", CSTR(node->get_name().operator lain::String()), "type", CSTR(node->get_class()));
					if (node->get_parent() != nullptr)
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
				{
					GObject* scene = memnew(TestNode);
					GObject* gobj1 = memnew(TestNode);
					GObject* gobj2 = memnew(TestNode);
					GObject* gobj1_1 = memnew(TestNode);
					scene->set_name("TestScene");
					scene->add_component(memnew(TestComponent));


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
					gobj1_1->add_component(memnew(TestComponent));
					Ref<PackedScene> s;
					s.instantiate();
					s->pack(scene);
					ResourceSaver::save(s, "1.tscn");
					s->pack(scene);
					ResourceSaver::save(s, "2.tscn");
				}
				Ref<PackedScene> s2 = ResourceLoader::load("2.tscn", "PackedScene");
				GObject* subscene = s2->instantiate();
				{
					Ref<PackedScene> s;
					s.instantiate();
					Ref<PackedScene> s1 = ResourceLoader::load("1.tscn", "PackedScene");
					GObject* newscene = s1->instantiate();
					L_JSON(newscene->get_components());

					L_PRINT(subscene->data.instance_state.is_null(), CSTR(subscene->data.scene_file_path));
					List<Ref<Resource>> resources;
					ResourceCache::get_cached_resources(&resources);
					for (Ref<Resource> i : resources) {
						L_JSON(i->GetPath());
						L_JSON(i->GetName());
					}

					newscene->add_child(subscene);
					subscene->set_owner(newscene);
					s->pack(newscene);
					draw_tree(newscene);
					// 查看是否reuse 可以

					ResourceSaver::save(s, "3.tscn");
				}
				List<Ref<Resource>> resources;
				ResourceCache::get_cached_resources(&resources);

				for (auto&& i : resources) {
					L_JSON(i->GetPath());
					L_JSON(i->GetName());
				}

				if (resources.is_empty()) {
					L_PRINT("this is empty");
				}
				Ref<PackedScene> s3 = ResourceLoader::load("3.tscn", "PackedScene");
				auto newscene_dup = s3->instantiate();
				draw_tree(newscene_dup);
				// @TODO 清理todo ext resource
				L_PRINT("---------");


			}
		}
	}
	
}