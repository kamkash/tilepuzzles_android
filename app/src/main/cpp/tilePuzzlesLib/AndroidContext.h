#ifndef TILEPUZZLES_ANDROIDCONTEXT_H
#define TILEPUZZLES_ANDROIDCONTEXT_H

#include <android/asset_manager.h>
#include "tilePuzzelsLib.h"

struct AndroidContext : GameContext {
    AAssetManager *assetManager;
};
#endif //TILEPUZZLES_ANDROIDCONTEXT_H
