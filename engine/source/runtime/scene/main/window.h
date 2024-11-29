#ifndef WINDOW_H
#define WINDOW_H

#include "scene/main/viewport.h"
namespace lain {
class Window : public Viewport {
  LCLASS(Window, Viewport);

 public:
  WindowSystem::WindowID window_id = WindowSystem::INVALID_WINDOW_ID;
  String title;
  mutable Vector2i position;
  mutable Size2i size = Size2i(DEFAULT_WINDOW_SIZE, DEFAULT_WINDOW_SIZE);
};
}  // namespace lain
#endif