#include "window.h"
#include "function/display/window_system.h"
using namespace lain;
void lain::Window::_notification(int what) {
  switch(what){

		case NOTIFICATION_ENTER_TREE: {
					window_id = WindowSystem::MAIN_WINDOW_ID;
          position = WindowSystem::GetSingleton()->window_get_position(window_id);
          size = WindowSystem::GetSingleton()->window_get_size(window_id);
          focused = WindowSystem::GetSingleton()->window_is_focused(window_id);

          L_CORE_PRINT("size:" + size.operator lain::String());
          L_CORE_PRINT("pos:" + position.operator lain::String());
          WindowSystem::GetSingleton()->registerInputEventCallback(
            std::bind(&Window::_window_input, this, std::placeholders::_1), window_id
          );
    }; break;

  }
}

void lain::Window::_window_input(const Ref<InputEvent>& p_ev) {
  
	ERR_MAIN_THREAD_GUARD;
  // debugger
  //

  if (is_inside_tree()) {
		push_input(p_ev);
	}
}

Window::Window() {
  // RS::get_singleton()->viewport_set_update_mode(get_viewport_rid(), RS::VIEWPORT_UPDATE_DISABLED);
}