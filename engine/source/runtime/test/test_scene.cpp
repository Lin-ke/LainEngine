#include "test_scene.h"
#include "../_generated/serializer/all_serializer.h"
#include "core/meta/serializer/serializer.h"
#include "core/scene/component/component.h"
#include "core/scene/packed_scene.h"
#include "core/scene/scene_tree.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/light_3d.h"
#include "scene/3d/camera_3d_data.h"
#include "scene/resources/io/resource_format_text.h"
#include "core/scene/component/component.h"
#include "core/scene/object/testnode.h"
#include "function/render/rendering_system/light_storage_api.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/resources/common/primitive_meshes.h"
#include "scene/3d/world_environment.h"
#include "scene/resources/common/sky_material.h"
#include "scene/resources/common/image_texture.h"
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
      Camera3D* cam = memnew(Camera3DMove);
      DirectionalLight3D* light = memnew(DirectionalLight3D);
      OmniLight3D* omni = memnew(OmniLight3D);
      WorldEnvironment* env = memnew(WorldEnvironment);
      Ref<Environment> ev;
      ev.instantiate();
      ev->set_background(Environment::BGMode::BG_SKY);
      Ref<Sky> sky;
      sky.instantiate();
      Ref<PanoramaSkyMaterial> material;
      material.instantiate();
      Ref<Image> img = ResourceLoader::load("res://lakeside_sunrise_4k.hdr");
      Ref<Texture2D> tex = ImageTexture::create_from_image(img);
      if(tex.is_null()){
        L_PRINT("error!!")
      }
      material->set_panorama(tex);
      sky->set_material(material);
      ev.instantiate();
      ev->set_sky(sky);
            
      scene->set_name("TestScene");
      scene->add_component(memnew(TestComponent));
      scene->add_child(cam);
      cam->set_owner(scene);
      cam->set_name("default_cam");
      cam->set_transform(
        Transform3D(Vector3(1,2,3), Vector3(1,2,3), Vector3(1,2,3), Vector3(1,2,3)));
      scene->add_child(env);
      env->set_owner(scene);
      env->set_name("default_env");
      env->set_environment(ev);

      omni->set_color(Color(.2, .1, 1));
      scene->add_child(omni);
      omni->set_owner(scene);
      omni->set_name("default_omni");
      light->set_color(Color(.5, .5, 1));
      light->set_shadow(true);
      scene->add_child(light);
      light->set_owner(scene);
      light->set_name("default_light");
      gobj1->set_name("hello");
      
      scene->add_child(gobj1);
      gobj1->set_owner(scene);

      gobj1_1->set_name("hello_child");
      gobj1->add_child(gobj1_1);
      gobj1_1->set_owner(scene);

      gobj2->set_name("world");
      scene->add_child(gobj2);
      gobj2->set_owner(scene);
      MeshInstance3D* cube = memnew(MeshInstance3D);
      cube->set_name("cube");
      scene->add_child(cube);
      cube->set_owner(scene);

      // Ref<CapsuleMesh> mesh = memnew(CapsuleMesh);
      // cube->set_mesh(mesh);
      // mesh->set_radius(1);
      // Ref<StandardMaterial3D> mat = memnew(StandardMaterial3D);
      // mesh->surface_set_material(0, mat);
      Ref<ArrayMesh> mesh = ResourceLoader::load("res://robot.obj");
      cube->set_mesh(mesh);
      /*SceneTree::get_singleton()->get_root()->add_child(gobj1);
					SceneTree::get_singleton()->get_root()->add_child(gobj2);*/
      gobj1_1->add_component(memnew(TestComponent));
      
      Ref<PackedScene> s;
      s.instantiate();
      List<PropertyInfo> props;
      (s->get_property_list(&props));
      for (auto&& i : props) {
        L_PRINT(i.name);
      }
      s->pack(scene);
      
      ResourceSaver::save(s, "1.tscn");
    }
    // Ref<PackedScene> s2 = ResourceLoader::load("2.tscn");
    // GObject* subscene = s2->instantiate();
    // {
    //   Ref<PackedScene> s;
    //   s.instantiate();
    //   Ref<PackedScene> s1 = ResourceLoader::load("1.tscn");
    //   GObject* newscene = s1->instantiate();
    //   L_JSON(newscene->get_components());
    //   /// 这个get说明get set 方法都能正常被call
    //   L_PRINT((newscene->get_gobject_or_null(String("./default_cam"))->get("transform")).operator String());
    //   L_PRINT(subscene->data.instance_state.is_null(), CSTR(subscene->data.scene_file_path));
    //   DirectionalLight3D* light =(DirectionalLight3D*) newscene->get_gobject_or_null(String("./default_light"));
    //   auto mode = light->get_shadow_mode();
    //   switch(mode){
    //     case DirectionalLight3D::ShadowMode::SHADOW_ORTHOGONAL:
    //       L_PRINT("SHADOW_ORTHOGONAL");
    //       break;
    //     case DirectionalLight3D::ShadowMode::SHADOW_PARALLEL_2_SPLITS:
    //       L_PRINT("SHADOW_PARALLEL_2_SPLITS");
    //       break;
    //     case DirectionalLight3D::ShadowMode::SHADOW_PARALLEL_4_SPLITS:
    //       L_PRINT("SHADOW_PARALLEL_4_SPLITS");
    //       break;
    //   }
    //   MeshInstance3D* cube = (MeshInstance3D*)newscene->get_gobject_or_null(String("./cube"));
    //   Ref<Resource> mesh = cube->get_mesh();
    //   L_PRINT(mesh->get_class());
    //   Ref<PrimitiveMesh > pm = mesh;
    //   Ref<Resource> mat = pm->get_material();
    //   L_PRINT(mat->get_class())
            
      
    //   List<Ref<Resource>> resources;
    //   ResourceCache::get_cached_resources(&resources);

    //   newscene->add_child(subscene);
    //   subscene->set_owner(newscene);
    //   s->pack(newscene);
    //   draw_tree(newscene);
    //   // 查看是否reuse 可以
    //   ResourceSaver::save(s, "3.tscn");
    // }
    // List<Ref<Resource>> resources;
    // ResourceCache::get_cached_resources(&resources);

    // for (auto&& i : resources) {
    //   L_JSON(i->GetPath());
    //   L_JSON(i->GetName());
    // }

    // if (resources.is_empty()) {
    //   L_PRINT("this is empty");
    // }
    // Ref<PackedScene> s3 = ResourceLoader::load("3.tscn");
    // auto newscene_dup = s3->instantiate();
    // draw_tree(newscene_dup);
    // // @TODO 清理todo ext resource
    // L_PRINT("---------");
  }
}
}  // namespace test

}  // namespace lain