#include <nlohmann/json.hpp>

#ifdef USE_SDL
#include "GLogger.h"
#endif
#include "TAppWin.h"
#include "tilePuzzelsLib.h"

#ifdef __ANDROID__
#include "AssetUtil.h"
#endif

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>


using namespace tilepuzzles;

#ifdef USE_SDL
static Logger L;
#endif
static TAppWin app;

GameContext *getContext() {
    return app.gameContext;
}

void initLogger() {
#ifdef USE_SDL
    Logger::initLogger("tilePuzzlesLib");
    L = Logger::getLogger();
#endif
}

void initApp() { app.init(); }

void init(GameContext *context) {
    app.gameContext = context;
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

