#ifndef _HEX_TILE_H_
#define _HEX_TILE_H_

#ifndef __ANDROID__

#include "GLogger.h"

#endif

#include "GeoUtil.h"
#include "Tile.h"
#include "Vertex.h"
#include "enums.h"

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/mathfwd.h>
#include <math/vec2.h>

namespace tilepuzzles {

struct HexTile : Tile {

    HexTile(const std::string &id, const Point &topLeft, const Size &size, TriangleVertices *pQuad,
            TriangleIndices *pIndices, int texIndex, float texWidth, int indexOffset,
            const math::int2 &gridCoord, int tileNum)
            : Tile(id), triangleVertices(pQuad), triangleIndices(pIndices) {
        this->topLeft = topLeft;
        this->size = size;
        this->gridCoord = gridCoord;
        this->tileNum = tileNum;
        initVertices(texIndex, texWidth);
        initIndices(indexOffset);
        // logVertices();
    }

    virtual void initIndices(int indexOffset) {
        (*triangleIndices)[0] = indexOffset;
        (*triangleIndices)[1] = indexOffset + 1;
        (*triangleIndices)[2] = indexOffset + 2;
    }

    bool shiftColumnGroup() {
        float shift = trunc(gridCoord.y / 3.);
        return ((int) shift % 2);
    }

    virtual void updateVertices() {
        if (shiftColumnGroup()) {
            topLeft[1] -= size[1];
        }

        const math::float3 tri[] = {
                {topLeft[0],                topLeft[1] - size[1], 0.},           // bottom left
                {topLeft[0] + size[0],      topLeft[1] - size[1], 0.}, // bottom right
                {topLeft[0] + .5 * size[0], topLeft[1],           0.}       // top
        };

        const math::float3 offset =
                -1. * math::float3({topLeft[0] + size[0] * .5, topLeft[1] - size[1], 0.});
        const math::float3 yoffset = {0., size[1], 0.};

        math::float3 invTri[] = {geo.rotate(tri[0], math::F_PI, {0., 0., 1.}, offset),
                                 geo.rotate(tri[1], math::F_PI, {0., 0., 1.}, offset),
                                 geo.rotate(tri[2], math::F_PI, {0., 0., 1.}, offset)};

        invTri[0] = geo.translate(invTri[0], yoffset);
        invTri[1] = geo.translate(invTri[1], yoffset);
        invTri[2] = geo.translate(invTri[2], yoffset);

        if (gridCoord.x % 2) {
            if (gridCoord.y % 3 == 0) {
                (*triangleVertices)[0].position = invTri[0];
                (*triangleVertices)[1].position = invTri[1];
                (*triangleVertices)[2].position = invTri[2];
            } else if (gridCoord.y % 3 == 1) {
                (*triangleVertices)[0].position = tri[0];
                (*triangleVertices)[1].position = tri[1];
                (*triangleVertices)[2].position = tri[2];
            } else {
                (*triangleVertices)[0].position = invTri[0];
                (*triangleVertices)[1].position = invTri[1];
                (*triangleVertices)[2].position = invTri[2];
            }
        } else {
            if (gridCoord.y % 3 == 0) {
                (*triangleVertices)[0].position = tri[0];
                (*triangleVertices)[1].position = tri[1];
                (*triangleVertices)[2].position = tri[2];
            } else if (gridCoord.y % 3 == 1) {
                (*triangleVertices)[0].position = invTri[0];
                (*triangleVertices)[1].position = invTri[1];
                (*triangleVertices)[2].position = invTri[2];
            } else {
                (*triangleVertices)[0].position = tri[0];
                (*triangleVertices)[1].position = tri[1];
                (*triangleVertices)[2].position = tri[2];
            }
        }
    }

    virtual void updateTexCoords(int texIndex, float texWidth) {
        texIndex = texIndex % 8;
        (*triangleVertices)[0].texCoords = {texWidth * texIndex, 0};
        (*triangleVertices)[1].texCoords = {texWidth * (texIndex + .9), 0};
        (*triangleVertices)[2].texCoords = {texWidth * (texIndex + .9), .4};
    }

    /* A utility function to calculate area of triangle formed by (x1, y1),
    (x2, y2) and (x3, y3) */
    float area(const math::float3 &v1, const math::float3 &v2, const math::float3 &v3) const {
        return abs((v1[0] * (v2[1] - v3[1]) + v2[0] * (v3[1] - v1[1]) + v3[0] * (v1[1] - v2[1])) /
                   2.0);
    }

    /* A function to check whether point P(x, y) lies inside the triangle formed
    by A(x1, y1), B(x2, y2) and C(x3, y3) */
    bool isInside(const math::float3 &v1, const math::float3 &v2, const math::float3 &v3,
                  const math::float3 &p) const {
        /* Calculate area of triangle ABC */
        float A = area(v1, v2, v3);

        /* Calculate area of triangle PBC */
        float A1 = area(p, v2, v3);

        /* Calculate area of triangle PAC */
        float A2 = area(v1, p, v3);

        /* Calculate area of triangle PAB */
        float A3 = area(v1, v2, p);

        /* Check if sum of A1, A2 and A3 is same as A */
        // return (A == A1 + A2 + A3);
        return abs(A - (A1 + A2 + A3)) <= EPS;
    }

    virtual bool onClick(const math::float2 &coord) const {
        math::float3 p3 = math::float3({coord[0], coord[1], 0.});
        return isInside((*triangleVertices)[0].position, (*triangleVertices)[1].position,
                        (*triangleVertices)[2].position, p3);
    }

    virtual void logVertices() const {
//    L.info("TileId", tileId, "isBlank", isBlank, "groupKey", groupKey);
//    L.info("Grid Coord", gridCoord.x, gridCoord.y);
        std::for_each(std::begin(*triangleVertices), std::end(*triangleVertices),
                      [](const Vertex &v) {
//      L.info("pos:", v.position[0], v.position[1], "texCoords:", v.texCoords[0], v.texCoords[1]);
                      });
    }

    void logIndices() const {
//    L.info("TileId", tileId, "isBlank", isBlank);
        std::for_each(std::begin(*triangleIndices), std::end(*triangleIndices),
                      [](const uint16_t &idx) {
                          //L.info("index:", idx);
                      });
    }

    virtual void rotateAtAnchor(math::float2 anch, float angle) {
        const math::float3 offset = -1. * math::float3({anch.x, anch.y, 0.});
        math::float3 rotTri[] = {
                geo.rotate((*triangleVertices)[0].position, angle, {0., 0., 1.}, offset),
                geo.rotate((*triangleVertices)[1].position, angle, {0., 0., 1.}, offset),
                geo.rotate((*triangleVertices)[2].position, angle, {0., 0., 1.}, offset)};
        (*triangleVertices)[0].position = rotTri[0];
        (*triangleVertices)[1].position = rotTri[1];
        (*triangleVertices)[2].position = rotTri[2];
    }

    bool inverted() {
        return (*triangleVertices)[2].position[1] < (*triangleVertices)[0].position[1];
    }

    bool hasVertex(const math::float2 &vert) {
        return (abs(getVert<0>().x - vert.x) <= EPS && abs(getVert<0>().y - vert.y) <= EPS) ||
               (abs(getVert<1>().x - vert.x) <= EPS && abs(getVert<1>().y - vert.y) <= EPS) ||
               (abs(getVert<2>().x - vert.x) <= EPS && abs(getVert<2>().y - vert.y) <= EPS);
    }

    template<int index>
    math::float3 getVert() {
        return (*triangleVertices)[index].position;
    }

    TriangleVertices *triangleVertices;
    TriangleIndices *triangleIndices;
#ifndef __ANDROID__
    constexpr static Logger
    L = Logger::getLogger();
#endif
    constexpr static float EPS = 0.001F;
    GeoUtil::GeoUtil geo;
    std::string groupKey;
}; // namespace tilepuzzles

} // namespace tilepuzzles

#endif
