#ifndef _SLIDER_RENDERER_H_
#define _SLIDER_RENDERER_H_

#include "App.h"

#ifdef USE_SDL
#include "GLogger.h"
#endif

#include "SliderMesh.h"
#include "TRenderer.h"
#include "Tile.h"

#include <functional>
#include <iostream>
#include <vector>

using namespace filament;
using namespace filament::math;

namespace tilepuzzles {

struct SliderRenderer : TRenderer<TQuadVertexBuffer, Tile> {

    SliderRenderer() {
        mesh = std::shared_ptr<Mesh<TQuadVertexBuffer, Tile>>(new SliderMesh());
    }

  virtual void onMouseMove(const float2& dragPosition) {
    }

  virtual Tile* onMouseUp(const float2& pos) {
        math::float3 clipCoord = normalizeViewCoord(pos);
    Tile* tile = mesh->hitTest(clipCoord);
        return tile;
    }

  virtual Tile* onMouseDown(const float2& pos) {
        math::float3 clipCoord = normalizeViewCoord(pos);
    Tile* tile = mesh->hitTest(clipCoord);
    if (tile && !readOnly) {
            mesh->slideTiles(*tile);
            needsDraw = true;
        }
        return tile;
    }

  virtual Tile* onRightMouseDown(const float2& viewCoord) {
    if (!readOnly) {
      mesh->shuffle();
      needsDraw = true;
    }
        math::float3 clipCoord = normalizeViewCoord(viewCoord);
    Tile* tile = mesh->hitTest(clipCoord);
        return tile;
    }

    virtual void initMesh() {
        mesh->init(CFG);
    }

  // static constexpr const char* CFG = R"({
  //   "type":"slider",
  //     "dimension": {
  //       "count": 24
  //     }    ,
  //   "border": {
  //     "top":1,
  //     "left":0,
  //     "width": 4,
  //     "height": 4
  //   }
  // })";

  static constexpr const char* CFG = R"({
    "type":"slider",
      "dimension": {
        "count":15 
      }
  })";
};

} // namespace tilepuzzles
#endif
