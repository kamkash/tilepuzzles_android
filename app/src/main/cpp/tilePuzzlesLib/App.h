#ifndef _APP_H_
#define _APP_H_

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/View.h>

using namespace filament;

namespace tilepuzzles {
struct App {
  View* view;
  Camera* camera;
};
} // namespace tilepuzzles
#endif