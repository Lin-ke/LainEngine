#ifndef WINDOW_H
#define WINDOW_H

#include "scene/main/viewport.h"
namespace lain {
class Window : public Viewport {
  LCLASS(Window, Viewport);
  protected:
    void _notification(int what);

 public:
  void _window_input(const Ref<InputEvent> &p_ev);
  WindowSystem::WindowID window_id = WindowSystem::INVALID_WINDOW_ID;
  String title;
  mutable Vector2i position;
  mutable Size2i size;
  mutable bool focused;
  Window();
  ~Window();
};
}  // namespace lain
#endif