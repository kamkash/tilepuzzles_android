#ifndef _SLIDER_MESH_H_
#define _SLIDER_MESH_H_

#ifdef USE_SDL
#include "GLogger.h"
#endif

#include "Mesh.h"
#include "Tile.h"

using namespace std;
using namespace filament;
using namespace filament::math;

namespace tilepuzzles {

struct SliderVertexBuffer : TQuadVertexBuffer {
    SliderVertexBuffer(int numQuads) : TQuadVertexBuffer(numQuads) {
    }

    virtual size_t getSize() {
        return sizeof(QuadVertices) * (numVertShapes - 1);
    }

    virtual size_t getIndexSize() {
        return sizeof(QuadIndices) * (numVertShapes - 1);
    }
};

struct SliderMesh : Mesh<TQuadVertexBuffer, Tile> {
    SliderMesh() {
    }

    virtual int getTileCount() {
        return configMgr.config["dimension"]["count"].get<int>() + 1;
    }

    virtual void initVertexBuffers() {
        vertexBufferBorder.reset(new TQuadVertexBuffer(4));
        const int tileCount = getTileCount();
        vertexBuffer.reset(new SliderVertexBuffer(tileCount));
    }

  virtual void init(const std::string& jsonStr) {
        Mesh::init(jsonStr);
        tiles.back().isBlank = true;
    }

  virtual Tile* const blankTile() {
    auto tileIter = std::find_if(tiles.begin(), tiles.end(), [](const Tile& t) { return t.isBlank; });
        if (tileIter != tiles.end()) {
            int index = std::distance(tiles.begin(), tileIter);
            return &tiles[index];
        } else {
            return nullptr;
        }
    }

  virtual void slideTiles(const Tile& tile) {
        auto tiles = tilesToSlide(tile);
    Tile* blank = blankTile();
    std::for_each(tiles.begin(), tiles.end(), [blank](Tile* t) { t->swap(blank); });
    }

  std::vector<Tile*> tilesToSlide(const Tile& tile) {
        Direction dir = canSlide(tile);
    Tile* blank = blankTile();
    auto sliderTiles = std::vector<Tile*>();
        switch (dir) {
            case Direction::left: {
                int row = blank->gridCoord.x;
                int tileCol = tile.gridCoord.y;
                int blankCol = blank->gridCoord.y;
                for (int c = blankCol + 1; c <= tileCol; ++c) {
                    sliderTiles.push_back(tileAt(row, c));
                }
                return sliderTiles;
            }
            case Direction::right: {
                int row = blank->gridCoord.x;
                int tileCol = tile.gridCoord.y;
                int blankCol = blank->gridCoord.y;
                for (int c = blankCol - 1; c >= tileCol; --c) {
                    sliderTiles.push_back(tileAt(row, c));
                }
                return sliderTiles;
            }
            case Direction::down: {
                int col = blank->gridCoord.y;
                int tileRow = tile.gridCoord.x;
                int blankRow = blank->gridCoord.x;
                for (int r = blankRow - 1; r >= tileRow; --r) {
                    sliderTiles.push_back(tileAt(r, col));
                }
                return sliderTiles;
            }
            case Direction::up: {
                int col = blank->gridCoord.y;
                int tileRow = tile.gridCoord.x;
                int blankRow = blank->gridCoord.x;
                for (int r = blankRow + 1; r <= tileRow; ++r) {
                    sliderTiles.push_back(tileAt(r, col));
                }
                return sliderTiles;
            }
            default:
                return sliderTiles;
        }
    }

  virtual Direction canSlide(const Tile& tile) {
        Direction res = Direction::none;
    Tile* blank = blankTile();
        if (blank) {
      res = blank->gridCoord.x == tile.gridCoord.x
                ? blank->gridCoord.y > tile.gridCoord.y ? Direction::right : Direction::left
                : blank->gridCoord.y == tile.gridCoord.y
                  ? blank->gridCoord.x > tile.gridCoord.x ? Direction::down : Direction::up
                  : Direction::none;
        }
        return res;
    }

#ifdef USE_SDL
    constexpr static Logger L = Logger::getLogger();
#endif
};
} // namespace tilepuzzles
#endif
