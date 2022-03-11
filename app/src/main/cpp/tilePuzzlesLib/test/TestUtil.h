#ifndef _TEST_UTIL_H_
#define _TEST_UTIL_H_

#include "GLogger.h"

namespace tilepuzzles {

struct TestUtil {
  static void init_test() {
    static bool inited = false;
    if (!inited) {
      Logger::initLogger("tests");
      inited = true;
    }
  }
};
} // namespace tilepuzzles

#endif