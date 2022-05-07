#ifndef _MESH_H_
#define _MESH_H_

#include "ConfigMgr.h"

#ifdef USE_SDL
#include "GLogger.h"
#endif

#include "GameUtil.h"
#include <cstdlib>
#include <ctime>

#include "AnchorTile.h"
#include "App.h"
#include "TVertexBuffer.h"
#include "Tile.h"
#include "TileGroup.h"
#include "Vertex.h"
#include "enums.h"

#include <filament/Viewport.h>
#include <math/mat4.h>

#include "GeoUtil.h"

using namespace std;
using namespace filament;
using namespace filament::math;

namespace tilepuzzles {

template <typename VB, typename T>

struct Mesh {

  Mesh() {
  }

  virtual ~Mesh() {
  }

  virtual void init(const std::string& jsonStr) {
    if (jsonStr.empty()) {
      configMgr.init();
    } else {
      configMgr.init(jsonStr);
    }
    initVertexBuffers();
    initTiles();
    initBorder();
  }

  virtual void initVertexBuffers() {
    vertexBufferBorder.reset(new VB(4));
    const int tileCount = getTileCount();
    vertexBuffer.reset(new VB(tileCount));
  }

  virtual T* const blankTile() {
    return nullptr;
  }

  virtual Direction canSlide(const T& tile) {
    return Direction::none;
  }

  virtual void slideTiles(const T& tile) {
  }

  virtual std::vector<T*> rollTiles(const T& tile, Direction dir) {
    return std::vector<T*>();
  }

  virtual void rotateTileGroup(TileGroup<T>& tileGroup, float angle) {
  }

  virtual void rollTileGroups(const TileGroup<T>& tileGroup, Direction dir) {
  }

  void logTiles() {
    std::for_each(std::begin(tiles), std::end(tiles), [](const T& t) {
      t.logVertices();
      t.logIndices();
    });
  }

  virtual T* tileAt(int row, int column) {
    auto tileIter = std::find_if(tiles.begin(), tiles.end(), [row, column](const T& t) {
      return row == t.gridCoord.x && column == t.gridCoord.y;
    });
    if (tileIter != tiles.end()) {
      return &*tileIter;
    } else {
      return nullptr;
    }
  }

  virtual TileGroup<T>* tileGroupAt(int row, int column) {
    auto tileIter =
      std::find_if(tileGroupAnchors.begin(), tileGroupAnchors.end(),
                   [row, column](const auto& t) { return row == t.gridCoord.x && column == t.gridCoord.y; });
    if (tileIter != tileGroupAnchors.end()) {
      return &*tileIter;
    } else {
      return nullptr;
    }
  }

  virtual void setTileGroupZCoord(TileGroup<T>& tileGroup, float zCoord) {
  }

  virtual T* hitTest(const math::float3& clipCoord) {
    auto tileIter = std::find_if(tiles.begin(), tiles.end(), [&clipCoord](const T& t) {
      return t.onClick({clipCoord.x, clipCoord.y});
    });
    if (tileIter != tiles.end()) {
      return &*tileIter;
    } else {
      return nullptr;
    }
  }

  virtual TileGroup<T>* hitTestAnchor(const math::float3& clipCoord) {
    auto iter =
      std::find_if(tileGroupAnchors.begin(), tileGroupAnchors.end(), [&clipCoord](const auto& anch) {
        math::float2 point = anch.anchorPoint;
        return abs(point.x - clipCoord.x) <= GeoUtil::EPS_6 && abs(point.y - clipCoord.y) <= GeoUtil::EPS_6;
      });
    if (iter != tileGroupAnchors.end()) {
      return &*iter;
    } else {
      return nullptr;
    }
  }

  math::float4 normalizeViewCoord(const App& app, const math::float2& viewCoord) const {
    math::mat4 projMat = app.camera->getProjectionMatrix();
    math::mat4 invProjMat = app.camera->inverseProjection(projMat);
    float width = float(app.view->getViewport().width);
    float height = float(app.view->getViewport().height);
    math::float4 normalizedView = {viewCoord.x * 2. / width - 1., viewCoord.y * -2. / height + 1., 0., 1.};
    math::float4 clipCoord = invProjMat * normalizedView;
    return clipCoord;
  }

  virtual int getTileCount() {
    return configMgr.config["dimension"]["count"].template get<int>();
  }

  virtual void initTiles() {
    const int tileCount = getTileCount();
    const int dim = sqrt(tileCount);
    const float texWidth = 32. / 1024.;

    const Size size = {(GameUtil::HIGH_X - GameUtil::LOW_X) / dim * GameUtil::TILE_SCALE_FACTOR,
                       (GameUtil::HIGH_Y - GameUtil::LOW_Y) / dim * GameUtil::TILE_SCALE_FACTOR};
    Point topLeft = {GameUtil::LOW_X, GameUtil::HIGH_Y};

    int t = 0;
    int indexOffset = 0;
    for (int r = 0; r < dim; ++r) {
      for (int c = 0; c < dim; ++c) {
        topLeft.y = GameUtil::HIGH_Y - r * size.y;
        topLeft.x = GameUtil::LOW_X + c * size.x;
        const std::string tileId = string("tile") + to_string(r) + to_string(c);
        T tile(tileId, topLeft, size, &vertexBuffer->get(t), &vertexBuffer->getIndex(t), t, texWidth,
               indexOffset, {r, c}, t + 1, GameUtil::TILE_DEPTH);
        tiles.push_back(tile);
        ++t;
        indexOffset += 4;
      }
    }
  }

  virtual void shuffle() {
    GameUtil::shuffle<T>(tiles);
  }

  bool hasBorder() {
    return configMgr.config["border"] != nullptr;
  }

  virtual void initBorder() {
    auto border = configMgr.config["border"];
    if (border != nullptr) {
      const int borderTop = border["top"].template get<int>();
      const int borderLeft = border["left"].template get<int>();
      const int borderWidth = border["width"].template get<int>();
      const int borderHeight = border["height"].template get<int>();
      const int tileCount = getTileCount();
      const int dim = sqrt(tileCount);
      const float texWidth = 30. / 60.;
      const Size size = {(GameUtil::HIGH_X - GameUtil::LOW_X) / dim * GameUtil::TILE_SCALE_FACTOR,
                         (GameUtil::HIGH_Y - GameUtil::LOW_Y) / dim * GameUtil::TILE_SCALE_FACTOR};
      const float borderThickness = size.x * .1f;
      const Size horzSize = {size.x * borderWidth, borderThickness};
      const Size vertSize = {borderThickness, size.y * borderHeight};
      Point topLeft = {GameUtil::LOW_X, GameUtil::HIGH_Y};

      // top
      topLeft.x = GameUtil::LOW_X + borderLeft * size.x;
      topLeft.y = GameUtil::HIGH_Y - borderTop * size.y;
      T topTile("borderTop", topLeft, horzSize, &vertexBufferBorder->get(0), &vertexBufferBorder->getIndex(0),
                0, texWidth, 0, {0, 0}, 1, GameUtil::BORDER_DEPTH);
      borderTiles.push_back(topTile);

      // bottom
      topLeft.x = GameUtil::LOW_X + borderLeft * size.x;
      topLeft.y = GameUtil::HIGH_Y - (borderTop * size.y) - (borderHeight * size.y) + borderThickness;
      T bottomTile("borderBottom", topLeft, horzSize, &vertexBufferBorder->get(1),
                   &vertexBufferBorder->getIndex(1), 0, texWidth, 4, {0, 0}, 2, GameUtil::BORDER_DEPTH);
      borderTiles.push_back(bottomTile);

      // left
      topLeft.x = GameUtil::LOW_X + borderLeft * size.x;
      topLeft.y = GameUtil::HIGH_Y - borderTop * size.y;
      T leftTile("borderLeft", topLeft, vertSize, &vertexBufferBorder->get(2),
                 &vertexBufferBorder->getIndex(2), 1, texWidth, 8, {0, 0}, 3, GameUtil::BORDER_DEPTH);
      borderTiles.push_back(leftTile);

      // right
      topLeft.x = GameUtil::LOW_X + (borderLeft * size.x) + (borderWidth * size.x) - borderThickness;
      topLeft.y = GameUtil::HIGH_Y - borderTop * size.y;
      T rightTile("borderRight", topLeft, vertSize, &vertexBufferBorder->get(3),
                  &vertexBufferBorder->getIndex(3), 1, texWidth, 12, {0, 0}, 4, GameUtil::BORDER_DEPTH);
      borderTiles.push_back(rightTile);
    }
  }

  TileGroup<T> nearestAnchorGroup(const math::float2& point) {
    auto init = TileGroup<T>({100., 100.}, std::vector<T>(), 0, {0, 0});
    auto res =
      std::reduce(tileGroupAnchors.begin(), tileGroupAnchors.end(), init, [&point, this](auto a, auto b) {
        math::float2 pointa = a.anchorPoint;
        math::float2 pointb = b.anchorPoint;
        float adist = GeoUtil::tdist({point.x, point.y, 0.}, {pointa.x, pointa.y, 0.});
        float bdist = GeoUtil::tdist({point.x, point.y, 0.}, {pointb.x, pointb.y, 0.});
        return adist < bdist ? a : b;
      });
    return res;
  }

  virtual void processAnchorGroups() {
  }

  virtual void orderAnchorGroups() {
  }

  virtual void collectAnchors() {
  }

  virtual void addAnchor(const math::float2& point, int row, int col) {
  }

  ConfigMgr configMgr;
  std::shared_ptr<VB> vertexBuffer;
  std::shared_ptr<VB> vertexBufferBorder;

  std::vector<T> tiles;
  std::vector<T> borderTiles;

  std::shared_ptr<TQuadVertexBuffer> vertexBufferAnchors;
  std::vector<AnchorTile> anchorTiles;
  std::vector<TileGroup<T>> tileGroupAnchors;

#ifdef USE_SDL
  Logger L;
#endif
};
} // namespace tilepuzzles
#endif
