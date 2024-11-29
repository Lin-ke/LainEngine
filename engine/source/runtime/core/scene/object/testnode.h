#ifndef TEST_NODE_H
#define TEST_NODE_H
#include "core/os/os.h"
#include "core/os/thread.h"
#include "core/scene/component/component.h"
#include "core/scene/object/gobject.h"
#include "testnode_data.h"
namespace lain {
class TestNode : public GObject {
  LCLASS(TestNode, GObject);
  int a;
  void _notification(int p_notification) {
    switch (p_notification) {
      case NOTIFICATION_PROCESS:
        L_PRINT("hello, my name is " + data.name + ", my father is " + (data.parent ? data.parent->get_name() : String("None")) + " I'm ticking in thread",
                Thread::get_caller_id());
        OS::GetSingleton()->DelayUsec(1000 * 3000);
        L_PRINT("------------------");
      default:
        return;
    }
  }

 public:
  TestNode() { tickdata.process = true; }
  void* get_instance_data() const override { return memnew(TestNodeData()); }
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
  
}  // namespace lain

#endif