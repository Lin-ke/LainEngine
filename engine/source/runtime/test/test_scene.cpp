#include "test_scene.h"
#include "../_generated/serializer/all_serializer.h"
#include "core/meta/serializer/serializer.h"
#include "core/scene/component/component.h"
#include "core/scene/packed_scene.h"
#include "core/scene/scene_tree.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/camera_3d_data.h"
#include "scene/resources/io/resource_format_text.h"
#include "core/scene/component/component.h"
#include "core/scene/object/testnode.h"
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
    L_JSON(ResourceLoader::type_to_loader_idx)
    L_JSON(ResourceLoader::ext_to_loader_idx)
		L_JSON(ResourceLoader::type_to_loader_idx["PackedScene"])
    {
      GObject* scene = memnew(TestNode);
      GObject* gobj1 = memnew(TestNode);
      GObject* gobj2 = memnew(TestNode);
      GObject* gobj1_1 = memnew(TestNode);
      GObject* cam = memnew(Camera3D);
      scene->set_name("TestScene");
      scene->add_component(memnew(TestComponent));
      scene->add_child(cam);
      cam->set_owner(scene);
      cam->set_name("default_cam");

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
    Ref<PackedScene> s2 = ResourceLoader::load("2.tscn");
    GObject* subscene = s2->instantiate();
    {
      Ref<PackedScene> s;
      s.instantiate();
      Ref<PackedScene> s1 = ResourceLoader::load("1.tscn");
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
    Ref<PackedScene> s3 = ResourceLoader::load("3.tscn");
    auto newscene_dup = s3->instantiate();
    draw_tree(newscene_dup);
    // @TODO 清理todo ext resource
    L_PRINT("---------");
  }
}
}  // namespace test

}  // namespace lain