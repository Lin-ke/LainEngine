#pragma once
#ifndef TEST_SCENE_H
#define TEST_SCENE_H
#include "core/scene/object/gobject.h"
#include "core/os/thread.h"
namespace lain {
	REFLECTION_TYPE(TestNode)

		CLASS(TestNode : public GObject, WhiteListFields) {
		REFLECTION_BODY(TestNode);
		LCLASS(TestNode, GObject);
		void _notification(int p_notification) {
			switch (p_notification) {
			case NOTIFICATION_PROCESS:
				L_PRINT("hello, my name is " + data.name + ", my father is " + (data.parent?data.parent->get_name() : String("None")) + " I'm ticking in thread", Thread::get_caller_id());
			default:
				return;
			}
		}
		public:
		TestNode() { tickdata.process = true; }
	};
	namespace test {
		

		void draw_tree(GObject* root);
		void test_scene();

	}
}
#endif