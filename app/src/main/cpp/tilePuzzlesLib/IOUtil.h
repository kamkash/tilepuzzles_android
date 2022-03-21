#ifndef _IO_UTIL_H_
#define _IO_UTIL_H_

#include <stdlib.h>
#include <fstream>
#include <time.h>
#include <utils/Path.h>
#include <vector>

#ifdef __ANDROID__
#include "android_debug.h"
#include "AndroidContext.h"
#include "AssetUtil.h"
#endif

#ifdef USE_SDL
#include <filamentapp/FilamentApp.h>
#endif

#include "tilePuzzelsLib.h"
#include <utils/Path.h>

using utils::Path;
namespace tilepuzzles {
namespace IOUtil {

#ifdef USE_SDL
  static Logger L;
#endif

struct img_data {
    int width;
    int height;
    int channels;
    unsigned char *data;
};

std::vector<unsigned char> loadBinaryAsset(const utils::Path &path) {
#ifdef USE_SDL
    std::ifstream input(path.c_str(), std::ios::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
    return std::move(buffer);
#else
    std::vector<uint8_t> buf;
    auto success = AssetReadFile(((AndroidContext *) getContext())->assetManager, path.c_str(), buf);
    LOGI("load asset: %s %d", path.c_str(), success);
    return std::move(buf);
#endif
}

IOUtil::img_data imageLoad(const char *path, int channels) {
    int w, h, n;
#ifdef USE_SDL
    unsigned char *data = stbi_load(path, &w, &h, &n, channels);
    if (data == nullptr) {
        IOUtil::L.error("The texture ", path, " could not be loaded");
        return  {0, 0, 0, nullptr};
    }
    return {w, h, n, data};
#else
    std::vector<unsigned char> res = IOUtil::loadBinaryAsset(path);
    if (res.empty()) {
        return {0, 0, 0, nullptr};
    }
    unsigned char *data = stbi_load_from_memory(res.data(), res.size(), &w, &h, &n, channels);
    return {w, h, n, data};
#endif
}

Path getTexturePath(const char *textureName) {
    Path path = std::string("textures/") + textureName;

#ifdef USE_SDL
    path = FilamentApp::getRootAssetsPath() + path;
#endif
    return path;
}

Path getMaterialPath(const char *materialName) {
    Path path = std::string("materials/") + materialName;

#ifdef USE_SDL
    path = FilamentApp::getRootAssetsPath() + path;
#endif
    return path;
}

} // namespace IOUtil
} // namespace tilepuzzles
#endif
