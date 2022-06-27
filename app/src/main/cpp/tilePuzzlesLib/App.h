#ifndef _APP_H_
#define _APP_H_

#include "Vertex.h"
#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/View.h>

using namespace filament;

namespace tilepuzzles {
struct App {
  Engine* engine;
  Scene* scene;
  Skybox* skybox;
  filament::Renderer* filaRenderer = nullptr;
  Rect viewportRect;
  ViewportLayout viewportLayout;
};
} // namespace tilepuzzles
#endif
