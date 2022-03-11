#ifndef _GAME_UTIL_H_
#define _GAME_UTIL_H_

#include <stdlib.h>
#include <time.h>
#include <vector>
#include "Tile.h"

namespace tilepuzzles {
namespace GameUtil {

void init() {
  std::srand(time(NULL));
}

int trand(int min, int max) {
  return rand() % max + min;
}

template<typename T>
void shuffle(std::vector<T>& tiles) {
  int n = tiles.size();
  for (int i = n - 1; i >= 1; --i) {
    int j = trand(0, i);
    Tile& tilei = tiles[i];
    Tile& tilej = tiles[j];
    tilei.swap(tilej);
  }
}


} // namespace GameUtil
} // namespace tilepuzzles
#endif
