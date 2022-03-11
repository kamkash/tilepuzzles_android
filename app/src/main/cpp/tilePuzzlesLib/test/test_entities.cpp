#define CATCH_CONFIG_PREFIX_ALL
#include "GLogger.h"
#include "SliderMesh.h"
#include "TestUtil.h"
#include "Tile.h"
#include "Vertex.h"
#include "tilePuzzelsLib.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>

CATCH_TEST_CASE("EntityCreation", "[entity_creation]") {

  tilepuzzles::TestUtil::init_test();
  tilepuzzles::Logger L;
  const float texWidth = 32. / 1024.;

  tilepuzzles::QuadVertices QUAD_VERTICES = {
    {{-1, -1, 0}, {-1, -1, 0}, {0, 0}},
    {{1, -1, 0}, {-1, -1, 0}, {1, 0}},
    {{-1, 1, 0}, {-1, -1, 0}, {0, 1}},
    {{1, 1, 0}, {-1, -1, 0}, {1, 1}},
  };

  tilepuzzles::QuadIndices QUAD_INDICES = {
    0, 1, 2, 3, 2, 1,
  };

  const auto cfg = R"({
    "type":"slider",
      "dimension": {
        "count": 15
      }    
  })";

  CATCH_SECTION("tile initialization") {
    tilepuzzles::Tile tile2("tile");
    CATCH_REQUIRE(tile2.tileId == "tile");
  }

  CATCH_SECTION("tile dimensions initialization") {
    tilepuzzles::Tile tile2("tile1", tilepuzzles::Point(0., 0.),
                            tilepuzzles::Size(.5, .5), &QUAD_VERTICES, &QUAD_INDICES, 0,
                            texWidth, 0, {0, 0}, 1);
    CATCH_REQUIRE(tile2.tileId == "tile1");
    CATCH_REQUIRE(tile2.topLeft == tilepuzzles::Point(0., 0.));
    CATCH_REQUIRE(tile2.size == tilepuzzles::Size(0.5, 0.5));
  }

  CATCH_SECTION("vertex buffer creation") {
    tilepuzzles::TQuadVertexBuffer vb(2);

    tilepuzzles::Tile tile1("tile1", tilepuzzles::Point(0., 0.),
                            tilepuzzles::Size(.5, .5), &vb.get(0), &vb.getIndex(0), 0,
                            texWidth, 0, {0, 0}, 1);
    tilepuzzles::Tile tile2("tile2", tilepuzzles::Point(0.1, 0.1),
                            tilepuzzles::Size(.1, .1), &vb.get(1), &vb.getIndex(1), 0,
                            texWidth, 1, {0, 0}, 2);

    CATCH_REQUIRE(vb.get(0)[0].position[0] == tile1.quadVertices[0]->position[0]);

    tile1.logVertices();
  }

  CATCH_SECTION("mesh creation") {

    const int count = 16;
    tilepuzzles::SliderMesh sliderMesh;
    sliderMesh.init(cfg);
    CATCH_REQUIRE(sliderMesh.tiles.size() == count);
    sliderMesh.logTiles();

    int index = count;
    tilepuzzles::QuadVertices* pQuad = sliderMesh.vertexBuffer->vertShapes;
    tilepuzzles::QuadIndices* pIndex = sliderMesh.vertexBuffer->indexShapes;
    while (index-- > 0) {
      L.info("****** quad", (*pQuad++)[0].position[0], *(*pIndex++));
    }
  }
}
