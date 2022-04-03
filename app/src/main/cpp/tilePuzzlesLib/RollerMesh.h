#ifndef _ROLLER_MESH_H_
#define _ROLLER_MESH_H_

#ifdef USE_SDL
#include "GLogger.h"
#endif
#include "Mesh.h"
#include "Tile.h"
#include "enums.h"

#include <limits>
using namespace std;
using namespace filament;
using namespace filament::math;

namespace tilepuzzles {

struct RollerMesh : Mesh<TQuadVertexBuffer, Tile> {
  RollerMesh() {
  }

  virtual void initTiles() {
    Mesh::initTiles();
    initTileBounds();
  }

  virtual std::vector<Tile*> rollTiles(const Tile& tile, Direction dir) {
    auto rollerTiles = tilesToRoll(tile, dir);
    const int dim = sqrt(tiles.size());
    std::for_each(rollerTiles.begin(), rollerTiles.end(),
                  [this, dir, dim](Tile* t) { rollTile(*t, dir, dim - 1); });

    return rollerTiles;
  }

  std::vector<Tile*> tilesToRoll(const Tile& tilePick, Direction dir) {
    const int pickRow = tilePick.gridCoord.x;
    const int pickCol = tilePick.gridCoord.y;
    const int dim = sqrt(tiles.size());
    auto rollerTiles = std::vector<Tile*>();
    switch (dir) {
      case Direction::down:
      case Direction::up: {
        for (int r = 0; r < dim; ++r) {
          rollerTiles.push_back(tileAt(r, pickCol));
        }
        return rollerTiles;
      }
      case Direction::left:
      case Direction::right: {
        for (int c = 0; c < dim; ++c) {
          rollerTiles.push_back(tileAt(pickRow, c));
        }
        return rollerTiles;
      }
      default:
        return rollerTiles;
    }
  }

  void rollTile(Tile& tile, Direction dir, int maxCoord) {
    switch (dir) {
      case Direction::up:
        tile.topLeft.y += tile.size.y;
        tile.gridCoord.x -= 1;
        if (tile.gridCoord.x < 0) {          
          tile.topLeft.y = low_y + tile.size.y;
          tile.gridCoord.x = maxCoord;
        }
        break;
      case Direction::down:
        tile.topLeft.y -= tile.size.y;
        tile.gridCoord.x += 1;
        if (tile.gridCoord.x > maxCoord) {
          tile.topLeft.y = high_y;
          tile.gridCoord.x = 0;
        }
        break;
      case Direction::left:
        tile.topLeft[0] -= tile.size.x;
        tile.gridCoord.y -= 1;
        if (tile.gridCoord.y < 0) {
          tile.topLeft.x = high_x - tile.size.x;
          tile.gridCoord.y = maxCoord;
        }
        break;
      case Direction::right:
        tile.topLeft.x += tile.size.x;
        tile.gridCoord.y += 1;
        if (tile.gridCoord.y > maxCoord) {
          tile.topLeft.x = low_x;
          tile.gridCoord.y = 0;
        }
        break;
      default:
        break;
    }
    tile.updateVertices();
  }

  void initTileBounds() {
    low_x = low_y = std::numeric_limits<float>::max();
    high_x = high_y = std::numeric_limits<float>::min();

    std::for_each(tiles.begin(), tiles.end(), [this](const Tile& t) {
      QuadVertices* iniQuad = t.iniQuadVertices;

      low_x = std::min(low_x, (*iniQuad)[0].position.x);
      low_x = std::min(low_x, (*iniQuad)[1].position.x);
      low_x = std::min(low_x, (*iniQuad)[2].position.x);
      low_x = std::min(low_x, (*iniQuad)[3].position.x);

      high_x = std::max(high_x, (*iniQuad)[0].position.x);
      high_x = std::max(high_x, (*iniQuad)[1].position.x);
      high_x = std::max(high_x, (*iniQuad)[2].position.x);
      high_x = std::max(high_x, (*iniQuad)[3].position.x);

      low_y = std::min(low_y, (*iniQuad)[0].position.y);
      low_y = std::min(low_y, (*iniQuad)[1].position.y);
      low_y = std::min(low_y, (*iniQuad)[2].position.y);
      low_y = std::min(low_y, (*iniQuad)[3].position.y);

      high_y = std::max(high_y, (*iniQuad)[0].position.y);
      high_y = std::max(high_y, (*iniQuad)[1].position.y);
      high_y = std::max(high_y, (*iniQuad)[2].position.y);
      high_y = std::max(high_y, (*iniQuad)[3].position.y);
    });
  }

  float low_x;
  float high_x;
  float low_y;
  float high_y;
};

} // namespace tilepuzzles
#endif
