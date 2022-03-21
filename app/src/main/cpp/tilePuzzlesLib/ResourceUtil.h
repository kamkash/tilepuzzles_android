#ifndef _RESOURCE_UTIL_H_
#define _RESOURCE_UTIL_H_

#include <fstream>  // std::ifstream
#include <iostream> // std::cout
#include <experimental/filesystem>

namespace tilepuzzles {
struct ResourceUtil {

    std::filesystem::path getCwd() {
        //
        return std::filesystem::current_path();
    }

    std::filesystem::path getResourcePath(const std::string resource) {
#ifdef USE_SDL
        std::filesystem::path wd = getCwd();
        wd /= resource;
        return wd;
#else
        return std::filesystem::path(resource);
#endif
    }

    void resourceAsString(const std::filesystem::path &resPath,
                          std::string &data) {
        std::ifstream inpStream(resPath.c_str(), std::ifstream::binary);
        if (inpStream) {
            long len = getStreamLength(inpStream);
            if (len > 0) {
                data.resize(len);
                inpStream.read(&data[0], len);
            }
        }
    }

    long getStreamLength(std::istream &is) {
        long length = 0L;
        is.seekg(0, is.end);
        length = is.tellg();
        is.seekg(0, is.beg);
        return length;
    }
};
} // namespace tilepuzzles
#endif