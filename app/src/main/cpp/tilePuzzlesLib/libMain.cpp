#include <nlohmann/json.hpp>

#ifndef __ANDROID__
#include "GLogger.h"
#endif

#include "AndroidContext.h"
#include "TAppWin.h"
#include "tilePuzzelsLib.h"
#include "AssetUtil.h"
#include "android_debug.h"

#ifdef __ANDROID__
#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

#endif

using namespace tilepuzzles;

#ifndef __ANDROID__
static Logger L;
#endif
static TAppWin app;

AndroidContext *getContext() {
    return app.androidContext;
}

void initLogger() {
#ifndef __ANDROID__
    Logger::initLogger("tilePuzzlesLib");
    L = Logger::getLogger();
#endif
}

void initApp() { app.init(); }

void init(AndroidContext *androidContext) {
    app.androidContext = androidContext;
    initLogger();
    initApp();
}

void destroy() {
    app.destroy_window();
}

void gameLoop(long frameTimeNanos) {
    app.game_loop((double) frameTimeNanos);
}

void touchAction(int action, float x, float y) {
    app.touchAction(action, x, y);
}

void render() {}

void destroySwapChain() {
    app.destroySwapChain();
}

void createSwapChain(void *nativeWin) {
    app.createSwapChain(nativeWin);
}

void resizeWindow(int width, int height) {
    app.resize_window(width, height);
}

