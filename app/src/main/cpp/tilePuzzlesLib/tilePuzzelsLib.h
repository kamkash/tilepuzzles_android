#ifndef TILEPUZZLESLIB_H
#define  TILEPUZZLESLIB_H

struct GameContext {
};

GameContext *getContext();

extern "C" {
void init(GameContext *gameContext);
void destroy();
void gameLoop(long frameTimeNanos);
void render();
void shuffle();
void destroySwapChain();
void createSwapChain(void *nativeWin);
void resizeWindow(int width, int height);
void touchAction(int action, float x, float y) ;
}
#endif

