#define CATCH_CONFIG_PREFIX_ALL

#include "ConfigMgr.h"
#include "ResourceUtil.h"
#include "tilePuzzelsLib.h"
#include <catch2/catch_test_macros.hpp>

#include "GLogger.h"
#include "TestUtil.h"

using namespace tilepuzzles;

CATCH_TEST_CASE("configuration", "[config]") {
  tilepuzzles::TestUtil::init_test();
  tilepuzzles::Logger L;
  ResourceUtil resUtil;

  CATCH_SECTION("current working directory") {
    std::filesystem::path cwd = resUtil.getCwd();
    CATCH_REQUIRE(std::string(cwd.c_str()).length() > 0);
    L.info("cwd:", cwd.c_str());
  }

  CATCH_SECTION("json config") {
    std::filesystem::path fpath =
      resUtil.getResourcePath("resources/config.json");
    CATCH_REQUIRE(std::string(fpath.c_str()).length() > 0);
    L.info("fpath:", fpath.c_str());

    std::string jsonStr;
    resUtil.resourceAsString(fpath, jsonStr);
    CATCH_REQUIRE(jsonStr.length() > 0);
    L.info("json string:", jsonStr);

    ConfigMgr cfgMgr(jsonStr);
    CATCH_REQUIRE(cfgMgr.config["type"] == "slider");
    CATCH_REQUIRE(cfgMgr.config["dimension"]["count"] == 15);
  }
}