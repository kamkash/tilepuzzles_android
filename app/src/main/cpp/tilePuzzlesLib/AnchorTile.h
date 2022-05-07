#ifndef _ANCHOR_TILE_H_
#define _ANCHOR_TILE_H_

#ifdef USE_SDL
#include "GLogger.h"
#else

#include "android_debug.h"

#endif

#ifdef USE_SDL
#include "GLogger.h"
#include <SDL.h>
#endif

#include "GameUtil.h"
#include "Vertex.h"
#include "enums.h"

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/mathfwd.h>
#include <math/vec2.h>

#include "TileDto.h"

namespace tilepuzzles {

struct AnchorTile : Tile {
  AnchorTile(const std::string& id, const Point& topLeft, const Size& size, QuadVertices* pQuad,
             QuadIndices* pIndices, int texIndex, float texWidth, int indexOffset,
             const math::int2& gridCoord, int tileNum, float depth)
    : Tile(id, topLeft, size, pQuad, pIndices, texIndex, texWidth, indexOffset, gridCoord, tileNum, depth) {
  }

  bool hasAnchorPoint(const math::float2& anchPoint) const {
    return abs(anchorPoint.x - anchPoint.x) <= EPS && abs(anchorPoint.y - anchPoint.y) <= EPS;
  }

  math::float2 anchorPoint;
};
} // namespace tilepuzzles
#endif