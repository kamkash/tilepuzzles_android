#ifndef _TAPPWIN_H_
#define _TAPPWIN_H_

#ifdef USE_SDL
#include "GLogger.h"
#include <SDL.h>
#endif

#include "App.h"
#include "GameUtil.h"
#include "HexSpinRenderer.h"
#include "IRenderer.h"
#include "RollerRenderer.h"
#include "SliderRenderer.h"
#include "TRenderer.h"
#include "Tile.h"

#include "generated/resources/resources.h"
#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/IndirectLight.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#ifdef USE_SDL
#include <filamentapp/FilamentApp.h>
#endif

#include <filamentapp/IBL.h>
#include <filamentapp/NativeWindowHelper.h>
#include <filameshio/MeshReader.h>
#include <math/mat4.h>
#include <stb_image.h>
#include <utils/EntityManager.h>
#include <utils/Panic.h>
#include <utils/Path.h>

#include <functional>
#include <iostream>
#include <valarray>

using namespace filament;
using utils::Entity;
using utils::EntityManager;
using utils::Path;

namespace tilepuzzles {

static constexpr int ACTION_DOWN = 0;
static constexpr int ACTION_UP = 1;
static constexpr int ACTION_MOVE = 2;

#ifdef USE_SDL
static constexpr uint32_t WINDOW_FLAGS = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
#endif

struct TAppWin {
  void destroySwapChain() {
    if (app.engine && swapChain) {
      app.engine->destroy(swapChain);
      swapChain = nullptr;
    }
  }

  void createSwapChain(void* nativeWindow) {
    swapChain = app.engine->createSwapChain(nativeWindow);
  }

  void init() {
    app.engine = Engine::create();
    app.filaRenderer = app.engine->createRenderer();
    // app.scene = app.engine->createScene();
    app.skybox =
      Skybox::Builder().showSun(true).color({0. / 255., 0. / 255., 0. / 255., 1.f}).build(*app.engine);
    // app.scene->setSkybox(app.skybox);
#ifdef USE_SDL
    ASSERT_POSTCONDITION(SDL_Init(SDL_INIT_EVENTS) == 0, "SDL_Init Failure");
    GameUtil::GameUtil::init();
    createRenderer();
    initRenderer();
    createWinow();
    setup_window();
    setup_animating_scene();
    game_loop(0.);
    cleanup();
#else
    GameUtil::GameUtil::init();
    createRenderer();
    initRenderer();
    setup_animating_scene();
#endif
  }

  void initRenderer() {
    app.viewportLayout = vpLayout;
    renderer->init(app);
    if (roRenderer != nullptr) {
      app.viewportLayout = roVpLayout;
      roRenderer->init(app);
    }
  }

  void initRenderer1() {
    app.viewportRect = vp;
    renderer->init(app);
    if (roRenderer != nullptr) {
      app.viewportRect = roVp;
      roRenderer->init(app);
    }
  }

  void createWinow() {
    auto title = std::string("Tile Puzzles");
#ifdef USE_SDL
    sdl_window = SDL_CreateWindow(title.c_str(), GameUtil::WINDOW_X_POS, GameUtil::WINDOW_Y_POS,
                                  GameUtil::WINDOW_WIDTH, GameUtil::WINDOW_HEIGHT, WINDOW_FLAGS);
#endif
  }

  GameContext* getContext() {
    return gameContext;
  }

  void cleanup() {
    renderer->destroy();
    if (roRenderer != nullptr) {
      roRenderer->destroy();
    }
    if (app.engine && swapChain) {
      app.engine->destroy(swapChain);
      swapChain = nullptr;
    }
    app.engine->destroy(app.skybox);
    // app.engine->destroy(app.scene);
    app.engine->destroy(app.filaRenderer);
    Engine::destroy(&app.engine);
    destroy_window();
#ifdef USE_SDL
    SDL_Quit();
#endif
  }

  void destroy_window() {
#ifdef USE_SDL
    SDL_DestroyWindow(sdl_window);
#endif
  }

  void setup_animating_scene() {
    renderer->draw();
    if (roRenderer)
      roRenderer->draw();
    needsDraw = true;
    onNewFrame = animation_new_frame;
  }

  void createRenderer() {
    // renderer = std::shared_ptr<IRenderer>(new SliderRenderer());
    // roRenderer = std::shared_ptr<IRenderer>(new SliderRenderer());
    // roRenderer->setReadOnly(true);

    // renderer = std::shared_ptr<IRenderer>(new RollerRenderer());
    // roRenderer = std::shared_ptr<IRenderer>(new RollerRenderer());
    // roRenderer->setReadOnly(true);

    renderer = std::shared_ptr<IRenderer>(new HexSpinRenderer());
    roRenderer = std::shared_ptr<IRenderer>(new HexSpinRenderer());
    roRenderer->setReadOnly(true);
  }

  static void animation_new_frame(TAppWin& win, double dt) {
    if (win.swapChain) {
      win.renderer->update(dt);
      win.renderer->animate(dt);
      if (win.roRenderer) {
        win.roRenderer->update(dt);
        win.roRenderer->animate(dt);
      }
      win.needsDraw = true;
    }
  }

  void setup_window() {
    void* nativeWindow;
    void* nativeSwapChain;
#ifdef USE_SDL
    nativeWindow = ::getNativeWindow(sdl_window);
    nativeSwapChain = nativeWindow;
#endif
#if defined(__APPLE__)
    void* metalLayer = nullptr;
    if (kBackend == filament::Engine::Backend::METAL) {
      metalLayer = setUpMetalLayer(nativeWindow);
      // The swap chain on Metal is a CAMetalLayer.
      nativeSwapChain = metalLayer;
    }
#if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
    if (kBackend == filament::Engine::Backend::VULKAN) {
      // We request a Metal layer for rendering via MoltenVK.
      setUpMetalLayer(nativeWindow);
    }
#endif
#endif
#ifdef USE_SDL
    createSwapChain(nativeSwapChain);
    int width, height;
    SDL_GetWindowSize(sdl_window, &width, &height);
    resize_window(width, height);
#endif
  }

  void resize_window(int width, int height) {
#if defined(__APPLE__)
    void* nativeWindow = ::getNativeWindow(sdl_window);
    if (kBackend == filament::Engine::Backend::METAL) {
      resizeMetalLayer(nativeWindow);
    }
#if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
    if (kBackend == filament::Engine::Backend::VULKAN) {
      resizeMetalLayer(nativeWindow);
    }
#endif
#endif
    renderer->resize(width, height);
    if (roRenderer)
      roRenderer->resize(width, height);
    needsDraw = true;
  }

  void touchAction(int action, float x, float y) {
    static bool buttonDown = false;
    switch (action) {
      case ACTION_DOWN: {
        math::float2 mouseDownPos = {x, y};
        renderer->onMouseDown(mouseDownPos);
        buttonDown = true;
        break;
      }
      case ACTION_UP: {
        math::float2 mouseUpPos = {x, y};
        renderer->onMouseUp(mouseUpPos);
        buttonDown = false;
        break;
      }
      case ACTION_MOVE: {
        if (buttonDown) {
          filament::math::float2 pos = {x, y};
          renderer->onMouseMove(pos);
        }
        break;
      }
    }
  }

#ifdef USE_SDL
  void game_loop(double t) {

    size_t nClosed = 0;
    SDL_Event event;
    Uint64 lastTime = 0;
    const Uint64 kCounterFrequency = SDL_GetPerformanceFrequency();
    Uint32 totalFrameTicks = 0;
    Uint32 totalFrames = 0;
    bool buttonDown = false;

    while (nClosed < 1) {
      totalFrames++;
      Uint64 startTicks = SDL_GetPerformanceCounter();

      if (!UTILS_HAS_THREADING) {
        if (app.engine) {
          app.engine->execute();
        }
      }
      while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
          case SDL_QUIT:
            nClosed = 1;
            break;

          case SDL_MOUSEMOTION:
            if (buttonDown) {
              filament::math::float2 pos = {float(event.motion.x), float(event.motion.y)};
              renderer->onMouseMove(pos);
            }
            break;

          case SDL_MOUSEBUTTONDOWN:

            switch (event.button.button) {
              case SDL_BUTTON_LEFT: {
                math::float2 mouseDownPos = {float(event.button.x), float(event.button.y)};
                renderer->onMouseDown(mouseDownPos);
                buttonDown = true;
                break;
              }

              case SDL_BUTTON_RIGHT:
                math::float2 mouseDownPos = {float(event.button.x), float(event.button.y)};
                renderer->onRightMouseDown(mouseDownPos);
                break;
            }
            break;

          case SDL_MOUSEBUTTONUP:
            switch (event.button.button) {
              case SDL_BUTTON_LEFT: {
                math::float2 mouseUpPos = {float(event.button.x), float(event.button.y)};
                renderer->onMouseUp(mouseUpPos);
                buttonDown = false;
                break;
              }

              case SDL_BUTTON_RIGHT:
                break;
            }
            break;

          case SDL_WINDOWEVENT:
            switch (event.window.event) {
              case SDL_WINDOWEVENT_RESIZED:
                if (event.window.windowID == SDL_GetWindowID(sdl_window)) {
                  resize_window(event.window.data1, event.window.data2);
                  break;
                }
                break;
              case SDL_WINDOWEVENT_CLOSE:
                if (event.window.windowID == SDL_GetWindowID(sdl_window)) {
                  SDL_HideWindow(sdl_window);
                  break;
                }
                nClosed++;
                break;
              default:
                break;
            }
            break;
          default:
            break;
        }
      }
      Uint64 endTicks = SDL_GetPerformanceCounter();
      const double dt = lastTime > 0 ? (double(endTicks - lastTime) / kCounterFrequency) : (1.0 / 60.0);
      lastTime = endTicks;

      time += dt;
      if (onNewFrame) {
        onNewFrame(*this, time);
      }

      if (!needsDraw) {
        continue;
      }

      if (app.filaRenderer->beginFrame(swapChain)) {
        app.filaRenderer->render(renderer->getView());
        if (roRenderer)
          app.filaRenderer->render(roRenderer->getView());
        app.filaRenderer->endFrame();
      }

      needsDraw = false;
      lastDrawTime = time;
      Uint64 frameTicks = endTicks - startTicks;
      float frameTime = (float)frameTicks / (float)kCounterFrequency;
      float delay = floor(16.666f - frameTime);
      SDL_Delay(delay);
    }
  }
#else

  void game_loop(double t) {
    if (renderer && swapChain) {
      if (onNewFrame) {
        onNewFrame(*this, t);
      }

      if (needsDraw) {
        if (app.filaRenderer->beginFrame(swapChain)) {
          app.filaRenderer->render(renderer->getView());
          if (roRenderer)
            app.filaRenderer->render(roRenderer->getView());
          app.filaRenderer->endFrame();
        }
        needsDraw = false;
        lastDrawTime = t;
      }
    }
  }

#endif

#if defined(__APPLE__)
  Engine::Backend kBackend = filament::Engine::Backend::METAL;
#endif

  std::function<void(TAppWin&, double)> onNewFrame;
#ifdef USE_SDL
  SDL_Window* sdl_window = nullptr;
#endif
  bool needsDraw = true;
  double time = 0.0;
  double lastDrawTime = 0.0;
  std::shared_ptr<IRenderer> renderer;
  std::shared_ptr<IRenderer> roRenderer;
  SwapChain* swapChain = nullptr;
  GameContext* gameContext;
  App app;
  Rect vp = {.topLeft = {320, 0}, .size = {320, 320}};
  Rect roVp = {.topLeft = {0., 320. - 240}, .size = {240, 240}};
  ViewportLayout vpLayout = {.dimFractions = {.5, 1.}, .offsetFractions = {0.5, 0.}};
  ViewportLayout roVpLayout = {.dimFractions = {.5, .5}, .offsetFractions = {0., 0.5}};
#ifdef USE_SDL
  Logger L;
#endif
};
} // namespace tilepuzzles
#endif
