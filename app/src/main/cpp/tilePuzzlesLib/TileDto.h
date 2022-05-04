#ifndef _HEX_TILE_DTO_H_
#define _HEX_TILE_DTO_H_

#ifdef USE_SDL
#include "GLogger.h"
#else
#include "android_debug.h"
#endif

#include "GeoUtil.h"
#include "Tile.h"
#include "Vertex.h"
#include "enums.h"

namespace tilepuzzles {

struct TileDto {
  Point topLeft;
  Size size;
  std::string tileId;
  math::int2 gridCoord = {0, 0};     
  std::unique_ptr<TriangleVertices> triangleVertices;
  std::unique_ptr<TriangleVertices> iniTriangleVertices;
  std::unique_ptr<TriangleIndices> triangleIndices;
};

} // namespace tilepuzzles

#endif
