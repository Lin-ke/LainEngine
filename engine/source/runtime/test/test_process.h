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
			
		}
	}
}
#endif
