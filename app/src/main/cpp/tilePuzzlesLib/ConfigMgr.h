#ifndef _CONFIG_MGR_H_
#define _CONFIG_MGR_H_

#include <nlohmann/json.hpp>

#ifndef __ANDROID__
#include "GLogger.h"
#endif

#include <filesystem>
#include "ResourceUtil.h"

using json = nlohmann::json;

namespace tilepuzzles {
struct ConfigMgr {
    ConfigMgr() {
    }

    void init() {
        ResourceUtil resUtil;

        std::filesystem::path fpath = resUtil.getResourcePath("resources/config.json");
        std::string jsonStr;
        resUtil.resourceAsString(fpath, jsonStr);
        init(jsonStr);
    }

    ConfigMgr(std::string &jsonStr) {
        init(jsonStr);
    }

    void init(const std::string &jsonStr) {
        config = json::parse(jsonStr);
    }

    json config;
#ifndef __ANDROID__
    constexpr static Logger L = Logger::getLogger();
#endif
};

} // namespace tilepuzzles
#endif