#ifndef _IRENDERER_H_
#define _IRENDERER_H_

#include "App.h"

namespace tilepuzzles {

struct IRenderer {
  IRenderer() {
  }

  virtual ~IRenderer() {
  }

  virtual Tile* onMouseDown(const float2& viewCoord) = 0;

  virtual Tile* onRightMouseDown(const float2& viewCoord) = 0;

  virtual Tile* onMouseUp(const float2& viewCoord) = 0;

  virtual void onMouseMove(const float2& dragPosition) = 0;

  virtual void initMesh() = 0;

  virtual void resize(int width, int height) = 0;

  virtual void init(const App& app) = 0;

  virtual void destroy() = 0;

  virtual void update(double dt) = 0;

  virtual void draw() = 0;

  virtual void drawBorder() = 0;

  virtual void animate(double now) = 0;

  virtual void shuffle() = 0;

  virtual bool isReadOnly() = 0;

  virtual void setReadOnly(bool readOnly) = 0;

  virtual View* getView() = 0;
};
} // namespace tilepuzzles
#endif
