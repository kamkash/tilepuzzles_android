#ifndef _VERTEX_H_
#define _VERTEX_H_

#include <filament/Texture.h>
#include <glog/logging.h>
#include <math/mathfwd.h>

using namespace filament;
namespace tilepuzzles {

// size 4*3 + 4*3 + 4*2 = 32 bytes
struct Vertex {
  filament::math::float3 position;
  filament::math::float3 normal;
  filament::math::float2 texCoords;
};
using QuadVertices = Vertex[4];
using TriangleVertices = Vertex[3];
using QuadIndices = uint16_t[6];
using TriangleIndices = uint16_t[3];
using Point = filament::math::float2;
using Size = filament::math::float2;

} // namespace tilepuzzles
#endif