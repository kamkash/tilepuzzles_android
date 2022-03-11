#ifndef _APP_H_
#define _APP_H_

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <utils/EntityManager.h>

namespace tilepuzzles {

struct FilamentAppObjects {
  VertexBuffer* vb;
  IndexBuffer* ib;
  Material* mat;
  MaterialInstance* matInstance;
  Camera* cam;
  Entity camera;
  Skybox* skybox;
  Texture* tex;
  Entity renderable;
};

} // namespace tilepuzzles

#endif