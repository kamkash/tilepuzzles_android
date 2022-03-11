#define CATCH_CONFIG_PREFIX_ALL
#include "GLogger.h"
#include "TestUtil.h"
#include "GeoUtil.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <numeric>
#include <math/mat3.h>
#include <math/mat4.h>
#include <math/mathfwd.h>
#include <math/vec2.h>

using namespace filament;
using namespace tilepuzzles;

#define LOG_VERT(label, arr)                                                                                 \
  std::for_each(std::begin(arr), std::end(arr),                                                              \
                [&L](const math::float3& f) { L.info(label, f[0], f[1], f[2]); });

CATCH_TEST_CASE("Geo", "[geo]") {
  Logger L;
  TestUtil::init_test();
  GeoUtil::GeoUtil geo;

  CATCH_SECTION("geo vec angle") {
    const math::float3 v1 = {0., 1., 0.};
    const math::float3 v2 = {.5, .5, 0.};
    const math::float3 v3 = {-.5, .5, 0.};

    float angle = geo.angleBetween(v1, v2);
    float angle1 = geo.angleBetween(v1, v3);
    math::float3 norm = geo.tcross(v1, v2);
    math::float3 norm1 = geo.tcross(v1, v3);
    L.info("angle", angle, angle * 180. / math::F_PI);
    L.info("tcross", norm[0], norm[1], norm[2]);
    L.info("tcross1", norm1[0], norm1[1], norm1[2]);
    L.info("angle1", angle1, angle1 * 180. / math::F_PI);
  }

  CATCH_SECTION("geo rotate") {
    const math::float3 tri[] = {{-1., 1., 0.}, {1., 1., 0.}, {0., 2.732, 0}};
    const math::float3 invTri[] = {geo.rotate(tri[0], math::F_PI, {0., 0., 1.}, {0., -tri[0][1], 0.}),
                                   geo.rotate(tri[1], math::F_PI, {0., 0., 1.}, {0., -tri[0][1], 0.}),
                                   geo.rotate(tri[2], math::F_PI, {0., 0., 1.}, {0., -tri[0][1], 0.})};

    LOG_VERT("tri", tri);
    LOG_VERT("invTri", invTri);
  }

  CATCH_SECTION("geo translate") {
    const math::float3 tri[] = {{-1., 1., 0.}, {1., 1., 0.}, {0., 2.732, 0}};
    const math::float3 transTri[] = {geo.translate(tri[0], {0., -tri[0][1], 0.}),
                                     geo.translate(tri[1], {0., -tri[0][1], 0.}),
                                     geo.translate(tri[2], {0., -tri[0][1], 0.})};

    LOG_VERT("tri", tri);
    LOG_VERT("transTri", transTri);
  }
}

CATCH_TEST_CASE("drag angle", "[dragAngle]") {
  tilepuzzles::Logger L;
  tilepuzzles::TestUtil::init_test();
  GeoUtil::GeoUtil geo;
  const math::float3 v1 = {.9, 0., 0.};
  const math::float3 v2 = {.2, .6, 0.};
  const math::float3 v3 = {.9, 0., 0.};

  math::float3 norm = geo.tcross(v1, v2);
  float angle = geo.angleBetween(v1, v2);
  L.info("norm", norm.x, norm.y, norm.z, "angle", angle);

  norm = geo.tcross(v1, v3);
  angle = geo.angleBetween(v1, v3);
  L.info("norm", norm.x, norm.y, norm.z, "angle", angle);

  float pcos1 = geo.tdot(v1,v2);
  float pcos2 = geo.tdot(v1,v3);
  L.info("pcos", pcos1, pcos2);
}

CATCH_TEST_CASE("drag angle", "[distance]") {
  tilepuzzles::Logger L;
  tilepuzzles::TestUtil::init_test();
  std::vector<int> ints = {3, 9, 1, 15};

  int res = std::reduce(ints.begin(), ints.end(), -1, [](int a, int b) {
    return std::fmax(a,b);
  });

  L.info("reduce", res);

}