#define CATCH_CONFIG_PREFIX_ALL
#include "GLogger.h"
#include "TLogger.h"
#include "TestUtil.h"
#include <catch2/catch_test_macros.hpp>
#include <glog/logging.h>
#include <iostream>

using namespace tilepuzzles;

CATCH_TEST_CASE("TestLogger", "[logger]") {
  tilepuzzles::TestUtil::init_test();
  tilepuzzles::Logger L;

  CATCH_SECTION("testing tlogger") {
    print_args("p1", "t1", "p2", "t2");
  }

  CATCH_SECTION("testing logger") {
    L.info("param1", "param2");
  }
}
