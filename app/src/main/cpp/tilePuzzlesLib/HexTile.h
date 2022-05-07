#ifndef _HEX_TILE_H_
#define _HEX_TILE_H_

#ifdef USE_SDL
#include "GLogger.h"
#else

#include "android_debug.h"

#endif

#include "GeoUtil.h"
#include "Tile.h"
#include "Vertex.h"
#include "enums.h"

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/mathfwd.h>
#include <math/vec2.h>

#include "TileDto.h"

namespace tilepuzzles {

struct HexTile : Tile {

  HexTile(const std::string& id, const Point& topLeft, const Size& size, TriangleVertices* pQuad,
          TriangleIndices* pIndices, int texIndex, float texWidth, int indexOffset,
          const math::int2& gridCoord, int tileNum, float depth)
    : Tile(id), triangleVertices(pQuad), triangleIndices(pIndices) {
    this->topLeft = topLeft;
    this->size = size;
    this->gridCoord = gridCoord;
    this->tileNum = tileNum;
    this->depth = depth;
    initVertices(texIndex, texWidth);
    initIndices(indexOffset);
  }

  HexTile(const std::string& id) : Tile(id) {
  }

  virtual void initIndices(int indexOffset) {
    (*triangleIndices)[0] = indexOffset;
    (*triangleIndices)[1] = indexOffset + 1;
    (*triangleIndices)[2] = indexOffset + 2;
  }

  bool shiftColumnGroup() {
    float shift = trunc(gridCoord.y / 3.);
    return ((int)shift % 2);
  }

  virtual void assign(HexTile* other) {
    topLeft = other->topLeft;
    gridCoord = other->gridCoord;

    (*triangleVertices)[0].position = (*other->triangleVertices)[0].position;
    (*triangleVertices)[1].position = (*other->triangleVertices)[1].position;
    (*triangleVertices)[2].position = (*other->triangleVertices)[2].position;

    (*iniTriangleVertices)[0].position = (*other->iniTriangleVertices)[0].position;
    (*iniTriangleVertices)[1].position = (*other->iniTriangleVertices)[1].position;
    (*iniTriangleVertices)[2].position = (*other->iniTriangleVertices)[2].position;
  }

  void assign(const TileDto& dto) {
    topLeft = dto.topLeft;
    gridCoord = dto.gridCoord;

    (*triangleVertices)[0].position = (*dto.triangleVertices)[0].position;
    (*triangleVertices)[1].position = (*dto.triangleVertices)[1].position;
    (*triangleVertices)[2].position = (*dto.triangleVertices)[2].position;

    (*iniTriangleVertices)[0].position = (*dto.iniTriangleVertices)[0].position;
    (*iniTriangleVertices)[1].position = (*dto.iniTriangleVertices)[1].position;
    (*iniTriangleVertices)[2].position = (*dto.iniTriangleVertices)[2].position;
  }

  virtual void translate(Direction dir, int rows, int columns) {
    float delta = 2. * size.y;
    float shift = 0.5 * size.y;

    (*triangleVertices)[0].position.y += delta;
    (*triangleVertices)[1].position.y += delta;
    (*triangleVertices)[2].position.y += delta;

    if ((*triangleVertices)[0].position.y > 1. || (*triangleVertices)[1].position.y > 1. ||
        (*triangleVertices)[2].position.y > 1.) {
      (*triangleVertices)[0].position.y = 1. - (*triangleVertices)[0].position.y - shift;
      (*triangleVertices)[1].position.y = 1. - (*triangleVertices)[1].position.y - shift;
      (*triangleVertices)[2].position.y = 1. - (*triangleVertices)[2].position.y - shift;
    }

    switch (dir) {
      case Direction::up: {
        gridCoord = {gridCoord.x, gridCoord.y - 1};
        if (gridCoord.y < 0) {
          gridCoord.y = rows - 1;
        }
        break;
      }
      case Direction::down: {
        gridCoord = {gridCoord.x, gridCoord.y + 1};
        if (gridCoord.y > rows - 1) {
          gridCoord.y = 0;
        }
        break;
      }
      case Direction::left: {
        gridCoord = {gridCoord.x - 1, gridCoord.y};
        if (gridCoord.x < 0) {
          gridCoord.x = columns - 1;
        }
        break;
      }
      case Direction::right: {
        gridCoord = {gridCoord.x + 1, gridCoord.y};
        if (gridCoord.x > columns - 1) {
          gridCoord.x = 0;
        }
        break;
      }
      default:
        break;
    }
  }

  virtual void updateNormals(const math::float3 norm) {
    (*triangleVertices)[0].normal = (*triangleVertices)[1].normal =
      (*triangleVertices)[2].normal = norm;
  }

  virtual void updateVertices() {
    if (shiftColumnGroup()) {
      topLeft[1] -= size[1];
    }

    const math::float3 tri[] = {
      {topLeft[0], topLeft[1] - size[1], depth},           // bottom left
      {topLeft[0] + size[0], topLeft[1] - size[1], depth}, // bottom right
      {topLeft[0] + .5 * size[0], topLeft[1], depth}       // top
    };

    const math::float3 offset = -1. * math::float3({topLeft[0] + size[0] * .5, topLeft[1] - size[1], depth});
    const math::float3 yoffset = {0., size[1], depth};

    math::float3 invTri[] = {GeoUtil::rotate(tri[0], math::F_PI, {0., 0., 1.}, offset),
                             GeoUtil::rotate(tri[1], math::F_PI, {0., 0., 1.}, offset),
                             GeoUtil::rotate(tri[2], math::F_PI, {0., 0., 1.}, offset)};

    invTri[0] = GeoUtil::translate(invTri[0], yoffset);
    invTri[1] = GeoUtil::translate(invTri[1], yoffset);
    invTri[2] = GeoUtil::translate(invTri[2], yoffset);

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

    if (iniTriangleVertices == nullptr) {
      iniTriangleVertices = (TriangleVertices*)malloc(sizeof(TriangleVertices));
      std::copy(std::begin(*triangleVertices), std::end(*triangleVertices), std::begin(*iniTriangleVertices));
    }
  }

  virtual void updateTexCoords(int texIndex, float texWidth) {
    (*triangleVertices)[0].texCoords = {texWidth * texIndex, 0};
    (*triangleVertices)[1].texCoords = {texWidth * (texIndex + .9), 0};
    (*triangleVertices)[2].texCoords = {texWidth * (texIndex + .9), .4};
  }

  virtual void setVertexZCoord(float zCoord) {
    (*triangleVertices)[0].position.z = (*triangleVertices)[1].position.z =
      (*triangleVertices)[2].position.z = zCoord;
    }

  /* A utility function to calculate area of triangle formed by (x1, y1),
  (x2, y2) and (x3, y3) */
  float area(const math::float3& v1, const math::float3& v2, const math::float3& v3) const {
    return abs((v1[0] * (v2[1] - v3[1]) + v2[0] * (v3[1] - v1[1]) + v3[0] * (v1[1] - v2[1])) / 2.0);
  }

  /* A function to check whether point P(x, y) lies inside the triangle formed
  by A(x1, y1), B(x2, y2) and C(x3, y3) */
  bool isInside(const math::float3& v1, const math::float3& v2, const math::float3& v3,
                const math::float3& p) const {
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

  virtual bool onClick(const math::float2& coord) const {
    math::float3 p3 = math::float3({coord[0], coord[1], 0.});
    return isInside((*triangleVertices)[0].position, (*triangleVertices)[1].position,
                    (*triangleVertices)[2].position, p3);
  }

  virtual void logVertices() const {
#ifdef USE_SDL
    L.info("TileId groupKey", tileId.c_str(), groupKey.c_str());
    L.info("Grid Coord:", gridCoord.x, gridCoord.y);
    std::for_each(std::begin(*triangleVertices), std::end(*triangleVertices),
                  [](const Vertex& v) { L.info("pos:", v.position[0], v.position[1]); });
#endif
  }

  void logIndices() const {
#ifdef USE_SDL
    L.info("TileId", tileId, "isBlank", isBlank);
    std::for_each(std::begin(*triangleIndices), std::end(*triangleIndices),
                  [](const uint16_t& idx) { L.info("index:", idx); });
#endif
  }

  virtual void rotateAtAnchor(math::float2 anch, float angle) {
    const math::float3 offset = -1. * math::float3({anch.x, anch.y, 0.});
    math::float3 rotTri[] = {GeoUtil::rotate((*triangleVertices)[0].position, angle, {0., 0., 1.}, offset),
                             GeoUtil::rotate((*triangleVertices)[1].position, angle, {0., 0., 1.}, offset),
                             GeoUtil::rotate((*triangleVertices)[2].position, angle, {0., 0., 1.}, offset)};
    (*triangleVertices)[0].position = rotTri[0];
    (*triangleVertices)[1].position = rotTri[1];
    (*triangleVertices)[2].position = rotTri[2];
  }

  bool inverted() {
    return (*triangleVertices)[2].position[1] < (*triangleVertices)[0].position[1];
  }

  virtual bool hasVertex(const math::float2& vert) {
    return (abs(getVert(0).x - vert.x) <= EPS && abs(getVert(0).y - vert.y) <= EPS) ||
           (abs(getVert(1).x - vert.x) <= EPS && abs(getVert(1).y - vert.y) <= EPS) ||
           (abs(getVert(2).x - vert.x) <= EPS && abs(getVert(2).y - vert.y) <= EPS);
  }

  virtual math::float3 getVert(int index) {
    return (*triangleVertices)[index].position;
  }

  std::unique_ptr<TriangleVertices> cloneTriangleVertices() const {
    TriangleVertices* clonedVertices = (TriangleVertices*)malloc(3 * sizeof(Vertex));
    memcpy(clonedVertices, triangleVertices, 3 * sizeof(Vertex));
    return std::unique_ptr<TriangleVertices>(clonedVertices);
  }

  std::unique_ptr<TriangleVertices> cloneIniTriangleVertices() const {
    TriangleVertices* clonedVertices = (TriangleVertices*)malloc(3 * sizeof(Vertex));
    memcpy(clonedVertices, iniTriangleVertices, 3 * sizeof(Vertex));
    return std::unique_ptr<TriangleVertices>(clonedVertices);
  }

  std::unique_ptr<TriangleIndices> cloneTriangleIndices() const {
    TriangleIndices* clonedIndices = (TriangleIndices*)malloc(3 * sizeof(uint16_t));
    memcpy(clonedIndices, triangleIndices, 3 * sizeof(uint16_t));
    return std::unique_ptr<TriangleIndices>(clonedIndices);
  }

  TileDto clone() const {
    TileDto dto;
    dto.triangleVertices = cloneTriangleVertices();
    dto.iniTriangleVertices = cloneIniTriangleVertices();
    dto.triangleIndices = cloneTriangleIndices();
    dto.size = size;
    dto.tileId = tileId;
    dto.gridCoord = gridCoord;
    return std::move(dto);
  }

  TriangleVertices* triangleVertices;
  TriangleIndices* triangleIndices;
  TriangleVertices* iniTriangleVertices = nullptr;
  std::string groupKey;
#ifndef __ANDROID__
  constexpr static Logger L = Logger::getLogger();
#endif
};

} // namespace tilepuzzles

#endif
