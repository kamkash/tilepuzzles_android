#ifndef _ROLLER_RENDERER_H_
#define _ROLLER_RENDERER_H_

#include "App.h"

#ifndef __ANDROID__
#include "GLogger.h"
#endif

#include "RollerMesh.h"
#include "TRenderer.h"
#include "Tile.h"

#include <iostream>
#include <thread>
#include <valarray>

using namespace filament;
using namespace filament::math;

namespace tilepuzzles {

struct RollerRenderer : TRenderer<TQuadVertexBuffer, Tile> {

    RollerRenderer() {
        mesh = std::shared_ptr<Mesh<TQuadVertexBuffer, Tile>>(new RollerMesh());
    }

    virtual void onMouseMove(const float2 &dragPosition) {
        math::float3 clipCoord = normalizeViewCoord(dragPosition);
        Tile *newTile = mesh->hitTest(clipCoord);
        if (newTile && !newTile->equals(dragTile)) {
            Direction dir = dragTile->directionTo(newTile);
            if (dir != Direction::none) {
                mesh->rollTiles(*dragTile, dir);
                needsDraw = true;
            }
        }
    }

    virtual Tile *onMouseDown(const math::float2 &pos) {
        math::float3 clipCoord = normalizeViewCoord(pos);
        dragTile = mesh->hitTest(clipCoord);
        return dragTile;
    }

    virtual Tile *onRightMouseDown(const float2 &viewCoord) {
        math::float3 clipCoord = normalizeViewCoord(viewCoord);
        Tile *tile = mesh->hitTest(clipCoord);
        return tile;
    }

    virtual Tile *onMouseUp(const math::float2 &pos) {
        dragTile = nullptr;
        math::float3 clipCoord = normalizeViewCoord(pos);
        Tile *tile = mesh->hitTest(clipCoord);
        return tile;
    }

    virtual void initMesh() {
        mesh->init(CFG);
    }

    static constexpr const char *CFG = R"({
    "type":"roller",
    "dimension": {
      "count": 25
    },
    "border": {
      "top":1,
      "left":1,
      "width": 4,
      "height": 4
    }
  })";

}; // namespace tilepuzzles

} // namespace tilepuzzles
#endif
