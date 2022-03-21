#ifndef _HEX_SPIN_MESH_H_
#define _HEX_SPIN_MESH_H_

#ifdef USE_SDL
#include "GLogger.h"
#endif

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

    virtual void init(const std::string &jsonStr) {
        Mesh::init(jsonStr);

        int anchCount = tileGroupAnchors.size();
        if (anchCount) {
            vertexBufferAnchors.reset(new TQuadVertexBuffer(anchCount));
            initAnchors();
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
        const float a = 2. / columns / 2.;
        const float h = sqrt3o2 * a;
        const Size size = {a, h};
        Point topLeft = {-1., 1.};

        int t = 0;
        for (int r = 0; r < rows * 2; ++r) {
            for (int c = 0; c < columns * 3; ++c) {
                int colGroup = trunc(c / 3);
                int rowGroup = trunc(r / 2);
                std::string key = to_string(rowGroup) + to_string(colGroup);

                topLeft.x = -1. + c * a * .5;
                topLeft.y = 1. - r * h;
                const std::string tileId = string("tile") + to_string(r) + to_string(c);
                HexTile tile(tileId, topLeft, size, &vertexBuffer->get(t),
                             &vertexBuffer->getIndex(t), (rowGroup * columns) + colGroup, texWidth,
                             indexOffset, {r, c}, t + 1);
                tile.groupKey = key;
                addTile(tile);
                addTileGroup(tile);
                indexOffset += 3;
                ++t;
            }
        }
        collectAnchors();
    }

    void addTile(const HexTile &tile) {
        tiles.push_back(tile);
    }

    void addTileGroup(const HexTile &tile) {
        if (tileGroups.find(tile.groupKey) == tileGroups.end()) {
            std::vector<HexTile> gtiles = {tile};
            tileGroups[tile.groupKey] = gtiles;
        } else {
            auto gTiles = tileGroups[tile.groupKey];
            gTiles.push_back(tile);
        }
    }

    void snapToPosition(const HexTile &tile) {
        auto grp = tileGroups[tile.groupKey];
        std::for_each(grp.begin(), grp.end(), [](auto tle) {

        });
    }

    void initAnchors() {
        int rows = configMgr.config["dimension"]["rows"].get<int>();
        int columns = configMgr.config["dimension"]["columns"].get<int>();
        const float a = 2. / columns / 2. / 4.;
        Size anchSize = {a, a};
        int anchIndex = 0;
        int indexOffset = 0;
        const float texWidth = 1.;
        std::for_each(tileGroupAnchors.begin(), tileGroupAnchors.end(),
                      [texWidth, &indexOffset, &anchIndex, anchSize, this](const auto &tileGroup) {
                          Point topLeft = {-1., 1.};
                          math::float2 anchPoint = std::get<0>(tileGroup);
                          topLeft.y = anchPoint.y + anchSize.y / 2.;
                          topLeft.x = anchPoint.x - anchSize.x / 2.;
                          const std::string tileId = string("anch") + to_string(anchIndex);
                          Tile tile(tileId, topLeft, anchSize, &vertexBufferAnchors->get(anchIndex),
                                    &vertexBufferAnchors->getIndex(anchIndex), 0, texWidth,
                                    indexOffset,
                                    {anchIndex, 0}, anchIndex + 1);
                          anchorTiles.push_back(tile);
                          ++anchIndex;
                          indexOffset += 4;
                      });
    }

    virtual void
    rotateTileGroup(const std::tuple<math::float2, std::vector<HexTile>> &tileGroup, float angle) {
        std::vector<HexTile> grp = std::get<1>(tileGroup);
        math::float2 pt = std::get<0>(tileGroup);
        std::for_each(grp.begin(), grp.end(),
                      [angle, &pt](HexTile &t) { t.rotateAtAnchor(pt, angle); });
    }

    virtual void shuffle() {
        float angle = GameUtil::trand(-60, 60);
    }

};

} // namespace tilepuzzles

#endif
