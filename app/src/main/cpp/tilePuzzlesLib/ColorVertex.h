#ifndef _COLOR_VERTEX_H_
#define _COLOR_VERTEX_H_

#include <filament/Texture.h>
#include <glog/logging.h>
#include <math/mathfwd.h>

using namespace filament;
namespace tilepuzzles {

// size 4*3 + 4*3 + 4 = 28 bytes
struct ColorVertex {
  filament::math::float3 position;
  filament::math::float3 normal;
  uint32_t color;
};
using QuadColorVertices = ColorVertex[4];
} // namespace tilepuzzles
#endif