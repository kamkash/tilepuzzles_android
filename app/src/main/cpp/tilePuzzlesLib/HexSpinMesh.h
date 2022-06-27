#ifndef _HEX_SPIN_MESH_H_
#define _HEX_SPIN_MESH_H_

#ifdef USE_SDL
#include "GLogger.h"
#endif

#include "AnchorTile.h"
#include "GameUtil.h"
#include "GeoUtil.h"
#include "HexTile.h"
#include "Mesh.h"
#include "TVertexBuffer.h"
#include "Vertex.h"
#include <tuple>

using namespace std;
using namespace filament;
using namespace filament::math;

namespace tilepuzzles {

struct HexSpinMesh : Mesh<TriangleVertexBuffer, HexTile> {
  HexSpinMesh() {
  }

  virtual void init(const std::string& jsonStr) {
    Mesh::init(jsonStr);

    int anchCount = tileGroupAnchors.size();
    if (anchCount) {
      vertexBufferAnchors.reset(new TQuadVertexBuffer(anchCount));
      initAnchors();
      updateDraggableAnchors();
    }
  }

  virtual int getTileCount() {
    int rows = configMgr.config["dimension"]["rows"].get<int>();
    int columns = configMgr.config["dimension"]["columns"].get<int>();
    return rows * 2 * columns * 3;
  }

  virtual void initVertexBuffers() {
    const int tileCount = getTileCount();
    vertexBuffer.reset(new TriangleVertexBuffer(tileCount));
  }

  virtual void initTiles() {
    const float sqrt3o2 = sqrt(3.) / 2.;
    const int rows = configMgr.config["dimension"]["rows"].get<int>();
    const int columns = configMgr.config["dimension"]["columns"].get<int>();
    const float texWidth = 32. / 1024.;
    int indexOffset = 0;
    const float a = ((GameUtil::HIGH_X - GameUtil::LOW_X) / columns / 2.) * GameUtil::TILE_SCALE_FACTOR;
    const float h = sqrt3o2 * a;
    const Size size = {a, h};
    Point topLeft = {GameUtil::LOW_X, GameUtil::HIGH_Y};

    int t = 0;
    for (int r = 0; r < rows * 2; ++r) {
      for (int c = 0; c < columns * 3; ++c) {
        int colGroup = trunc(c / 3);
        int rowGroup = trunc(r / 2);
        std::string key = to_string(rowGroup) + to_string(colGroup);

        topLeft.x = GameUtil::LOW_X + c * a * .5;
        topLeft.y = GameUtil::HIGH_Y - r * h;
        const std::string tileId = string("tile") + to_string(r) + to_string(c);
        HexTile tile(tileId, topLeft, size, &vertexBuffer->get(t), &vertexBuffer->getIndex(t),
                     (rowGroup * columns) + colGroup, texWidth, indexOffset, {r, c}, t + 1,
                     GameUtil::TILE_DEPTH);
        tile.groupKey = key;
        addTile(tile);
        indexOffset += 3;
        ++t;
      }
    }
    processAnchorGroups();
  }

  virtual void processAnchorGroups() {
    collectAnchors();
    orderAnchorGroups();
  }

  virtual void collectAnchors() {
    tileGroupAnchors.clear();
    Size size = tiles[0].size;
    int rows = (GameUtil::HIGH_Y - GameUtil::LOW_Y) / size.y;
    int columns = (GameUtil::HIGH_X - GameUtil::LOW_X) / size.x;
    rows *= 2;    // two rows per group
    columns *= 3; // three columns per group
    math::float2 point;
    for (int r = 0; r < rows; ++r) {
      for (int c = 0; c < columns; ++c) {
        point.x = GameUtil::LOW_X + c * size.x * .5;
        point.y = GameUtil::HIGH_Y - r * size.y;
        addAnchor(point, r, c);
      }
    }
  }

  virtual void addAnchor(const math::float2& point, int row, int col) {
    std::vector<HexTile> anchTiles;
    std::copy_if(tiles.begin(), tiles.end(), std::back_inserter(anchTiles),
                 [&point](HexTile& t) { return t.hasVertex(point); });
    if (anchTiles.size() == 6) {
      bool canDrag = false;
      if (row % 2) {
        canDrag = col == 2 || col == 8;
      } else {
        canDrag = col == 5 || col == 11;
      }
      int colGroup = trunc(col / 3);
      int rowGroup = trunc(row / 2);
      if (colGroup % 2) {
        rowGroup -= 1;
      }
      colGroup = canDrag ? colGroup : -1;
      rowGroup = canDrag ? rowGroup : -1;
      TileGroup<HexTile> t(point, anchTiles, canDrag, {rowGroup, colGroup});
      tileGroupAnchors.push_back(t);
    }
  }

  virtual void orderAnchorGroups() {
    const float sqrt3o2 = sqrt(3.) / 2.;
    const int rows = configMgr.config["dimension"]["rows"].get<int>();
    const int columns = configMgr.config["dimension"]["columns"].get<int>();
    const float a = ((GameUtil::HIGH_X - GameUtil::LOW_X) / columns / 2.) * GameUtil::TILE_SCALE_FACTOR;
    const float h = sqrt3o2 * a;
    const Size size = {a, h};
    int dragable = 0;
    for (auto& group : tileGroupAnchors) {
      if (group.dragable) {
        std::vector<HexTile> tileGroup = std::vector<HexTile>();
        math::float2 pt = group.anchorPoint;

        // top half
        float yCoord = pt.y + size.y * .5;
        math::float3 tileCenter = {pt.x - size.x * .5, yCoord, 0.};
        HexTile* tile = hitTest(tileCenter);
        if (tile) {
          tileGroup.push_back(*tile);
        }

        tileCenter = {pt.x, yCoord, 0.};
        tile = hitTest(tileCenter);
        if (tile) {
          tileGroup.push_back(*tile);
        }

        tileCenter = {pt.x + size.x * .5, yCoord, 0.};
        tile = hitTest(tileCenter);
        if (tile) {
          tileGroup.push_back(*tile);
        }

        // bottom half
        yCoord = pt.y - size.y * .5;
        tileCenter = {pt.x - size.x * .5, yCoord, 0.};
        tile = hitTest(tileCenter);
        if (tile) {
          tileGroup.push_back(*tile);
        }

        tileCenter = {pt.x, yCoord, 0.};
        tile = hitTest(tileCenter);
        if (tile) {
          tileGroup.push_back(*tile);
        }

        tileCenter = {pt.x + size.x * .5, yCoord, 0.};
        tile = hitTest(tileCenter);
        if (tile) {
          tileGroup.push_back(*tile);
        }
        group.tileGroup = tileGroup;
      }
    }
  }

  void addTile(const HexTile& tile) {
    tiles.push_back(tile);
  }

  void initAnchors() {
    int rows = configMgr.config["dimension"]["rows"].get<int>();
    int columns = configMgr.config["dimension"]["columns"].get<int>();
    const int tileCount = getTileCount();
    const int dim = sqrt(tileCount);
    const Size tileSize = {(GameUtil::HIGH_X - GameUtil::LOW_X) / dim * GameUtil::TILE_SCALE_FACTOR,
                           (GameUtil::HIGH_Y - GameUtil::LOW_Y) / dim * GameUtil::TILE_SCALE_FACTOR};
    const float a = tileSize.x / 1.5F;
    Size anchSize = {a, a};
    int anchIndex = 0;
    int indexOffset = 0;
    const float texWidth = 1.;
    std::for_each(tileGroupAnchors.begin(), tileGroupAnchors.end(),
                  [texWidth, &indexOffset, &anchIndex, anchSize, this](const auto& tileGroup) {
                    Point topLeft = {-1., 1.};
                    math::float2 anchPoint = tileGroup.anchorPoint;
                    topLeft.y = anchPoint.y + anchSize.y / 2.;
                    topLeft.x = anchPoint.x - anchSize.x / 2.;
                    const std::string tileId = string("anch") + to_string(anchIndex);
                    AnchorTile tile(tileId, topLeft, anchSize, &vertexBufferAnchors->get(anchIndex),
                              &vertexBufferAnchors->getIndex(anchIndex), 0, texWidth, indexOffset,
                              {anchIndex, 0}, anchIndex + 1, GameUtil::ANCHOR_DEPTH);
                    tile.anchorPoint = anchPoint;
                    anchorTiles.push_back(tile);
                    ++anchIndex;
                    indexOffset += 4;
                  });
  }

  void updateDraggableAnchors() {
    std::for_each(tileGroupAnchors.begin(), tileGroupAnchors.end(), [this](const auto& tileGroup) {
      if (tileGroup.dragable) {
        AnchorTile* anchTile = anchorTileAt(tileGroup.anchorPoint);
        if (anchTile) {
          anchTile->updateNormals({1.0F, 1.0F, 1.0F});
        }
      }
    });
  }

  AnchorTile* anchorTileAt(const math::float2& anchorPoint) {
    auto tileIter = std::find_if(anchorTiles.begin(), anchorTiles.end(),
                                 [&anchorPoint](const auto& t) { return t.hasAnchorPoint(anchorPoint); });

    if (tileIter != anchorTiles.end()) {
      return &*tileIter;
    } else {
      return nullptr;
    }
  }

  virtual void rotateTileGroup(TileGroup<HexTile>& tileGroup, float angle) {
    math::float2 pt = tileGroup.anchorPoint;
    std::for_each(tileGroup.tileGroup.begin(), tileGroup.tileGroup.end(),
                  [this, angle, &pt](HexTile& t) { t.rotateAtAnchor(pt, angle); });
  }

  virtual void setTileGroupZCoord(TileGroup<HexTile>& tileGroup, float zCoord) {
    std::for_each(tileGroup.tileGroup.begin(), tileGroup.tileGroup.end(),
                  [zCoord](HexTile& t) { t.setVertexZCoord(zCoord); });
  }

  virtual void shuffle() {
    int anchCount = tileGroupAnchors.size();
    for (int i = 0; i < HexSpinMesh::SHUFFLE_PASSES; ++i) {
      float angle = GameUtil::coinFlip() ? GeoUtil::PI_3 : -GeoUtil::PI_3;
      int anchIndex = GameUtil::trand(0, anchCount);
      auto anchor = tileGroupAnchors[anchIndex];
      rotateTileGroup(anchor, angle);
      processAnchorGroups();
    }
  }

  std::vector<TileGroup<HexTile>*> tileGroupsToRoll(const TileGroup<HexTile>& groupPick, Direction dir) {
    const int rows = configMgr.config["dimension"]["rows"].get<int>();
    const int columns = configMgr.config["dimension"]["columns"].get<int>();
    auto rollerGroups = std::vector<TileGroup<HexTile>*>();
    const int pickRow = groupPick.gridCoord.x;
    const int pickCol = groupPick.gridCoord.y;
    switch (dir) {
      case Direction::down:
      case Direction::up: {
        for (int r = 0; r < rows; ++r) {
          rollerGroups.push_back(tileGroupAt(r, pickCol));
        }
        return rollerGroups;
      }
      case Direction::left:
      case Direction::right: {
        for (int c = 0; c < columns; ++c) {
          rollerGroups.push_back(tileGroupAt(pickRow, c));
        }
        return rollerGroups;
      }
      default:
        return rollerGroups;
    }
  }

  virtual void rollTileGroups(const TileGroup<HexTile>& tileGroup, Direction dir) {
    std::vector<TileGroup<HexTile>*> rollerGroups = tileGroupsToRoll(tileGroup, dir);

    std::vector<TileDto> grp0;
    if (dir == Direction::down || dir == Direction::right) {
      grp0 = cloneTileGroup(*rollerGroups[0]);
      for (int i = 0; i < rollerGroups.size() - 1; ++i) {
        assignTileGroup(*rollerGroups[i + 1], *rollerGroups[i]);
      }
      assignTileGroup(grp0, *rollerGroups[rollerGroups.size() - 1]);
    } else {
      grp0 = cloneTileGroup(*rollerGroups[rollerGroups.size() - 1]);
      for (int i = rollerGroups.size() - 2; i >= 0; --i) {
        assignTileGroup(*rollerGroups[i], *rollerGroups[i + 1]);
      }
      assignTileGroup(grp0, *rollerGroups[0]);
    }
    processAnchorGroups();
  }

  std::vector<TileDto> cloneTileGroup(const TileGroup<HexTile>& srcGrp) {
    std::vector<TileDto> grp;
    std::transform(srcGrp.tileGroup.begin(), srcGrp.tileGroup.end(), std::back_inserter(grp),
                   [](const HexTile& hexTile) {
                     TileDto dto = hexTile.clone();
                     return dto;
                   });

    return grp;
  }

  virtual void rollTileGroups1(const TileGroup<HexTile>& tileGroup, Direction dir) {
    std::vector<TileGroup<HexTile>*> rollerGroups = tileGroupsToRoll(tileGroup, dir);
    std::for_each(rollerGroups.begin(), rollerGroups.end(), [this, dir](TileGroup<HexTile>* g) {
      if (g) {
        translateTileGroup(g->tileGroup, dir);
      }
    });
    processAnchorGroups();
  }

  void translateTileGroup(std::vector<HexTile>& shiftTiles, Direction dir) {
    const int rows = configMgr.config["dimension"]["rows"].get<int>();
    const int columns = configMgr.config["dimension"]["columns"].get<int>();
    std::for_each(shiftTiles.begin(), shiftTiles.end(), [dir, rows, columns](auto& t) {
      //
      t.translate(dir, rows, columns);
    });
  }

  void assignTileGroup(const TileGroup<HexTile>& srcGroup, const TileGroup<HexTile>& dstGroup) {
    std::vector<HexTile> srcTiles = srcGroup.tileGroup;
    std::vector<HexTile> dstTiles = dstGroup.tileGroup;
    for (int i = 0; i < srcTiles.size(); ++i) {
      auto src = srcTiles[i];
      auto dst = dstTiles[i];
      dst.assign(&src);
    }
  }

  void assignTileGroup(const std::vector<TileDto>& srcGroup, const TileGroup<HexTile>& dstGroup) {
    std::vector<HexTile> dstTiles = dstGroup.tileGroup;
    for (int i = 0; i < srcGroup.size(); ++i) {
      const TileDto& src = srcGroup[i];
      HexTile& dst = dstTiles[i];
      dst.assign(src);
    }
  }

  static constexpr int SHUFFLE_PASSES = 400;
};

} // namespace tilepuzzles

#endif
