#pragma once
#ifndef TEST_SCENE_H
#define TEST_SCENE_H
#include "core/scene/object/gobject.h"
#include "core/scene/packed_scene.h"
#include "resource/io/resource_format_text.h"
#include "core/scene/component/component.h"
namespace lain {
	namespace test {
		void test_scene() {
			GObject* gobj1 = memnew(GObject);
			GObject* gobj2 = memnew(GObject);
			gobj1->set_name("hello");
			gobj2->set_name("hello_child");
			gobj1->add_child(gobj2);
			gobj2->set_owner(gobj1);
			gobj2->add_component(memnew(Component));
			Ref<PackedScene> s;
			s.instantiate();
			s->pack(gobj1);
			ResourceSaver::save(s, "1.tscn");
			Ref<Resource> s1 = ResourceLoader::load("1.tscn","PackedScene");
			Ref<PackedScene> p = s1;
			GObject* gobj11 = p->instantiate();

		}

	}
}
#endif