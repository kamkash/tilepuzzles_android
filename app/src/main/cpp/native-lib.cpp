#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>
#include <AndroidContext.h>
#include <tilePuzzelsLib.h>

static AndroidContext androidContext;

extern "C" JNIEXPORT jstring JNICALL
Java_net_kamkash_tilepuzzles_MainActivity_stringFromJNI(JNIEnv *env, jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_net_kamkash_tilepuzzles_MainActivity_createSwapChain(JNIEnv *env, jobject /* this */,
                                                          jobject surface) {
    ANativeWindow *win = ANativeWindow_fromSurface(env, surface);
    createSwapChain(win);
}

extern "C" JNIEXPORT void JNICALL
Java_net_kamkash_tilepuzzles_MainActivity_destroySwapChain(JNIEnv *env, jobject /* this */) {
    destroySwapChain();
}

extern "C" JNIEXPORT void JNICALL
Java_net_kamkash_tilepuzzles_MainActivity_resizeWindow(JNIEnv *env, jobject /* this */, jint width,
                                                       jint height) {
    resizeWindow(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_net_kamkash_tilepuzzles_MainActivity_init(JNIEnv *env, jobject /* this */, jobject jAssetMgr) {
    androidContext.assetManager = AAssetManager_fromJava(env, jAssetMgr);
    init((GameContext *) &androidContext);
}

extern "C" JNIEXPORT void JNICALL
Java_net_kamkash_tilepuzzles_MainActivity_destroy(JNIEnv *env, jobject /* this */) {
    destroy();
}

extern "C" JNIEXPORT void JNICALL
Java_net_kamkash_tilepuzzles_MainActivity_gameLoop(JNIEnv *env, jobject /* this */,
                                                   jlong frameTimeNanos) {
    gameLoop(frameTimeNanos);
}

extern "C"
JNIEXPORT void JNICALL
Java_net_kamkash_tilepuzzles_MainActivity_touchAction(JNIEnv *env, jobject thiz, jint action,
                                                      jfloat raw_x, jfloat raw_y) {
    touchAction(action, raw_x, raw_y);
}