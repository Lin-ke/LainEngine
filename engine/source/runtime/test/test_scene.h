#pragma once
#ifndef TEST_SCENE_H
#define TEST_SCENE_H
#include "core/scene/object/gobject.h"
#include "core/os/thread.h"
#include "core/os/os.h"
#include "core/scene/component/component.h"
namespace lain {
	REFLECTION_TYPE(TestNode)

		CLASS(TestNode : public GObject, WhiteListFields) {
		REFLECTION_BODY(TestNode);
		LCLASS(TestNode, GObject);
		void _notification(int p_notification) {
			switch (p_notification) {
			case NOTIFICATION_PROCESS:
				L_PRINT("hello, my name is " + data.name + ", my father is " + (data.parent?data.parent->get_name() : String("None")) + " I'm ticking in thread", Thread::get_caller_id());
				OS::GetSingleton()->DelayUsec(1000 *  3000);
				L_PRINT("------------------");
			default:
				return;
			}
			
		}
		public:
		TestNode() { tickdata.process = true; }
	};
	REFLECTION_TYPE(TestComponent)
		CLASS(TestComponent : public Component, WhiteListFields) {
		REFLECTION_BODY(TestComponent);
		LCLASS(TestComponent, Component);
		void _notification(int p_notification) {
			switch (p_notification) {
			case GObject::NOTIFICATION_PROCESS:
				L_PRINT("hello, my name is " + get_class() + ", my father is " + (m_parent ? m_parent->get_name() : String("None")) + " I'm ticking in thread", Thread::get_caller_id());
				OS::GetSingleton()->DelayUsec(1000 * 3000);
				L_PRINT("------------------");
			default:
				return;
			}

		}
public:
	TestComponent() { tickdata.process = true; }
	};
	namespace test {
		

		void draw_tree(GObject* root);
		void test_scene();

	}
}
#endif