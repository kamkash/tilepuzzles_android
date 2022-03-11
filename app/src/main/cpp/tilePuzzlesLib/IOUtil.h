#ifndef _IO_UTIL_H_
#define _IO_UTIL_H_

#include <stdlib.h>
#include <fstream>
#include <time.h>
#include <utils/Path.h>
#include <vector>

#include "android_debug.h"
#include "AssetUtil.h"
#include "tilePuzzelsLib.h"

namespace tilepuzzles {
namespace IOUtil {

std::vector<unsigned char> loadBinaryAsset(const utils::Path &path) {
    std::ifstream input(path.c_str(), std::ios::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
    return std::move(buffer);
}

std::vector<unsigned char> loadAndroidBinaryAsset(const utils::Path &path) {
    std::vector<uint8_t> buf;
    auto success = AssetReadFile(getContext()->assetManager, path.c_str(), buf);
    LOGI("load asset: %s %d", path.c_str(), success);
    return std::move(buf);
}


} // namespace IOUtil
} // namespace tilepuzzles
#endif