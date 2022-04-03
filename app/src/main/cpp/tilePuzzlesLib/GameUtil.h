#ifndef _GAME_UTIL_H_
#define _GAME_UTIL_H_

#include <stdlib.h>
#include <time.h>
#include <vector>

namespace tilepuzzles {

struct GameUtil {

  static void init() {
    std::srand(time(NULL));
  }

  static int trand(int min, int max) {
    return rand() % max + min;
  }

  static float frand(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>((float)RAND_MAX / (max - min)));
  }

  static bool coinFlip() {
    return frand(0., 1.) > 0.5;
  }

  template <typename T>
  static void shuffle(std::vector<T>& tiles) {
    int n = tiles.size();
    for (int i = n - 1; i >= 1; --i) {
      int j = trand(0, i);
      T& tilei = tiles[i];
      T& tilej = tiles[j];
      tilei.swap(tilej);
    }
  }

  static constexpr float LOW_X = -1.F;
  static constexpr float HIGH_X = 1.F;
  static constexpr float LOW_Y = -1.F;
  static constexpr float HIGH_Y = .7F;
  static constexpr float TILE_SCALE_FACTOR = .6F;
};

} // namespace tilepuzzles
#endif
