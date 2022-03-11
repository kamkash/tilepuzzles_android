extern "C" {
void init(AndroidContext *androidContext);
void destroy();
void gameLoop(long frameTimeNanos);
void render();
void destroySwapChain();
void createSwapChain(void *nativeWin);
void resizeWindow(int width, int height);
void touchAction(int action, float x, float y) ;
}

AndroidContext *getContext();
