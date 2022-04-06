#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <stb_image.h>

#include "App.h"
#include "IOUtil.h"
#include "IRenderer.h"
#include "Mesh.h"
#include "Tile.h"

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
#include <math/mat4.h>
#include <utils/EntityManager.h>
#include <utils/Panic.h>
#include <utils/Path.h>

using namespace filament;
using namespace filament::math;
using utils::Entity;
using utils::EntityManager;
using utils::Path;
using MinFilter = TextureSampler::MinFilter;
using MagFilter = TextureSampler::MagFilter;

static constexpr Engine::Backend kBackend = Engine::Backend::OPENGL;

namespace tilepuzzles {
template <typename VB, typename T>
struct TRenderer : IRenderer {
  TRenderer() {
  }

  virtual ~TRenderer() {
  }

  virtual T* onMouseDown(const float2& viewCoord) = 0;

  virtual T* onRightMouseDown(const float2& viewCoord) = 0;

  virtual T* onMouseUp(const float2& viewCoord) = 0;

  virtual void onMouseMove(const float2& dragPosition) = 0;

  virtual void initMesh() = 0;

  virtual void filaRender() {
    if (filaRenderer->beginFrame(swapChain)) {
      filaRenderer->render(view);
      filaRenderer->endFrame();
    }
  }

  virtual void resize(int width, int height) {
    if (width > 0 && height > 0) {
      view->setViewport({0, 0, uint32_t(width), uint32_t(height)});
      const float aspect = getAspectRatio();

      if (aspect < 1.) {
        zoom /= aspect;
      }
      camera->setProjection(Camera::Projection::ORTHO, -aspect * zoom, aspect * zoom, -zoom, zoom, kNearPlane,
                            kFarPlane);
    }
  }

  virtual void executeEngine() {
    if (engine) {
      engine->execute();
    }
  }

  virtual void init() {
    initMesh();
    engine = Engine::create();
    filaRenderer = engine->createRenderer();
    utils::EntityManager& em = utils::EntityManager::get();
    em.create(1, &cameraEntity);
    camera = engine->createCamera(cameraEntity);
    view = engine->createView();
    view->setAmbientOcclusionOptions({.enabled = true});
    view->setBloomOptions({.enabled = true});
    view->setCamera(camera);
    scene = engine->createScene();
    view->setScene(scene);
    app.camera = camera;
    app.view = view;
  }

  virtual void destroySwapChain() {
    if (engine && swapChain) {
      engine->destroy(swapChain);
      swapChain = nullptr;
    }
  }

  virtual Path getTileMaterialPath() {
    return IOUtil::getMaterialPath(FILAMAT_FILE_UNLIT.data());
  }

  virtual void destroy() {
    engine->destroy(bgRenderable);
    engine->destroy(bgMatInstance);
    engine->destroy(bgTex);
    engine->destroy(bgVb);
    engine->destroy(bgIb);
    engine->destroy(bgMaterial);

    if (mesh->hasBorder()) {
      engine->destroy(borderRenderable);
      engine->destroy(borderMatInstance);
      engine->destroy(borderTex);
      engine->destroy(borderVb);
      engine->destroy(borderIb);
      engine->destroy(borderMaterial);
    }

    engine->destroy(skybox);
    engine->destroy(renderable);
    engine->destroy(matInstance);
    engine->destroy(material);
    engine->destroy(vb);
    engine->destroy(ib);
    engine->destroy(light);
    engine->destroy(pointLight);

    view->setScene(nullptr);
    engine->destroy(tex);
    engine->destroy(scene);
    engine->destroy(view);
    engine->destroyCameraComponent(cameraEntity);
    engine->destroy(cameraEntity);
    engine->destroy(swapChain);
    engine->destroy(filaRenderer);
    Engine::destroy(&engine);
  }

  float getAspectRatio() {
    float aspectRation = 1.F;
    uint32_t width = view->getViewport().width;
    uint32_t height = view->getViewport().height;
    if (width && height) {
      aspectRation = float(width) / float(height);
    }
    return aspectRation;
  }

  virtual SwapChain* createSwapChain(void* nativeSwapChain) {
    swapChain = engine->createSwapChain(nativeSwapChain);
    return swapChain;
  }

  virtual void update(double dt) {
    if (needsDraw) {
      needsDraw = false;
      vb->setBufferAt(*engine, 0,
                      VertexBuffer::BufferDescriptor(mesh->vertexBuffer->cloneVertices(),
                                                     mesh->vertexBuffer->getSize(),
                                                     (VertexBuffer::BufferDescriptor::Callback)free));
      scene->remove(renderable);
      auto& rcm = engine->getRenderableManager();
      rcm.destroy(renderable);

      RenderableManager::Builder(1)
        .boundingBox({{-1, -1, -1}, {1, 1, 1}})
        .material(0, matInstance)
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb, ib, 0, mesh->vertexBuffer->numIndices)
        .receiveShadows(false)
        .castShadows(false)
        .build(*engine, renderable);
      scene->addEntity(renderable);
    }
  }

  virtual void drawBackground() {
    static const Vertex QUAD_VERTICES[4] = {
      {{-1, -1, GameUtil::BACKGROUND_DEPTH}, {0, 0, 0}, {0, 0}},
      {{1, -1, GameUtil::BACKGROUND_DEPTH}, {0, 0, 0}, {1, 0}},
      {{-1, 1, GameUtil::BACKGROUND_DEPTH}, {0, 0, 0}, {0, 1}},
      {{1, 1, GameUtil::BACKGROUND_DEPTH}, {0, 0, 0}, {1, 1}},
    };

    static constexpr uint16_t QUAD_INDICES[6] = {
      0, 1, 2, 3, 2, 1,
    };

    Path path = getBackgroundTexturePath();
    IOUtil::img_data data = IOUtil::imageLoad(path.c_str(), 4);
    Texture::PixelBufferDescriptor buffer(data.data, size_t(data.width * data.height * 4),
                                          Texture::Format::RGBA, Texture::Type::UBYTE,
                                          (Texture::PixelBufferDescriptor::Callback) & ::stbi_image_free);

    static_assert(sizeof(Vertex) == (4 * 3) + (4 * 3) + (4 * 2), "Strange vertex size.");
    bgTex = Texture::Builder()
              .width(uint32_t(data.width))
              .height(uint32_t(data.height))
              .levels(1)
              .sampler(Texture::Sampler::SAMPLER_2D)
              .format(Texture::InternalFormat::RGBA8)
              .build(*engine);
    bgTex->setImage(*engine, 0, std::move(buffer));
    TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);
    // Create quad renderable
    bgVb = VertexBuffer::Builder()
             .vertexCount(4)
             .bufferCount(1)
             .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0, 32)
             .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT3, 24, 32)
             .build(*engine);
    bgVb->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(QUAD_VERTICES, sizeof(Vertex) * 4, nullptr));
    bgIb = IndexBuffer::Builder().indexCount(6).bufferType(IndexBuffer::IndexType::USHORT).build(*engine);
    bgIb->setBuffer(*engine, IndexBuffer::BufferDescriptor(QUAD_INDICES, sizeof(uint16_t) * 6, nullptr));

    Path matPath = IOUtil::getMaterialPath(FILAMAT_FILE_OPAQUE.data());
    std::vector<unsigned char> mat = IOUtil::loadBinaryAsset(matPath.c_str());
    bgMaterial = Material::Builder().package(mat.data(), mat.size()).build(*engine);

    bgMatInstance = bgMaterial->createInstance();
    bgMatInstance->setParameter("albedo", bgTex, sampler);
    bgRenderable = EntityManager::get().create();
    RenderableManager::Builder(1)
      .boundingBox({{-1, -1, -1}, {1, 1, 1}})
      .material(0, bgMatInstance)
      .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, bgVb, bgIb, 0, 6)
      .receiveShadows(false)
      .castShadows(false)
      .build(*engine, bgRenderable);
    scene->addEntity(bgRenderable);
  }

  virtual void drawBorder() {
    if (mesh->hasBorder()) {
      std::shared_ptr<VB> vbBorder = mesh->vertexBufferBorder;
      Path path = getBorderTexturePath();
      IOUtil::img_data data = IOUtil::imageLoad(path.c_str(), 4);
      Texture::PixelBufferDescriptor buffer(data.data, size_t(data.width * data.height * 4),
                                            Texture::Format::RGBA, Texture::Type::UBYTE,
                                            (Texture::PixelBufferDescriptor::Callback) & ::stbi_image_free);

      static_assert(sizeof(Vertex) == (4 * 3) + (4 * 3) + (4 * 2), "Strange vertex size.");
      borderTex = Texture::Builder()
                    .width(uint32_t(data.width))
                    .height(uint32_t(data.height))
                    .levels(1)
                    .sampler(Texture::Sampler::SAMPLER_2D)
                    .format(Texture::InternalFormat::RGBA8)
                    .build(*engine);
      borderTex->setImage(*engine, 0, std::move(buffer));
      TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);
      // Create quad renderable
      borderVb = VertexBuffer::Builder()
                   .vertexCount(vbBorder->numVertices)
                   .bufferCount(1)
                   .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0, 32)
                   .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT3, 24, 32)
                   .build(*engine);
      borderVb->setBufferAt(
        *engine, 0, VertexBuffer::BufferDescriptor(vbBorder->vertShapes, vbBorder->getSize(), nullptr));
      borderIb = IndexBuffer::Builder()
                   .indexCount(vbBorder->numIndices)
                   .bufferType(IndexBuffer::IndexType::USHORT)
                   .build(*engine);
      borderIb->setBuffer(
        *engine, IndexBuffer::BufferDescriptor(vbBorder->indexShapes, vbBorder->getIndexSize(), nullptr));

      Path matPath = IOUtil::getMaterialPath(FILAMAT_FILE_UNLIT.data());
      std::vector<unsigned char> mat = IOUtil::loadBinaryAsset(matPath.c_str());
      borderMaterial = Material::Builder().package(mat.data(), mat.size()).build(*engine);

      borderMatInstance = borderMaterial->createInstance();
      borderMatInstance->setParameter("alpha", 1.f);

      borderMatInstance->setParameter("albedo", borderTex, sampler);
      borderRenderable = EntityManager::get().create();
      RenderableManager::Builder(1)
        .boundingBox({{-1, -1, -1}, {1, 1, 1}})
        .material(0, borderMatInstance)
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, borderVb, borderIb, 0, vbBorder->numIndices)
        .receiveShadows(false)
        .castShadows(false)
        .build(*engine, borderRenderable);
      scene->addEntity(borderRenderable);
    }
  }

  virtual void draw() {
    drawBackground();
    drawTiles();
    drawBorder();
    // addLight();
  }

  virtual Path getTilesTexturePath() {
    Path path = IOUtil::getTexturePath("1-30c.png");
    return path;
  }

  virtual Path getBorderTexturePath() {
    Path path = IOUtil::getTexturePath("border2.png");
    return path;
  }

  virtual Path getBackgroundTexturePath() {
    Path path = IOUtil::getTexturePath("wood.jpeg");
    return path;
  }

  void drawTiles() {
    Path path = getTilesTexturePath();
    IOUtil::img_data data = IOUtil::imageLoad(path.c_str(), 4);
    Texture::PixelBufferDescriptor buffer(data.data, size_t(data.width * data.height * 4),
                                          Texture::Format::RGBA, Texture::Type::UBYTE,
                                          (Texture::PixelBufferDescriptor::Callback) & ::stbi_image_free);

    static_assert(sizeof(Vertex) == (4 * 3) + (4 * 3) + (4 * 2), "Strange vertex size.");
    tex = Texture::Builder()
            .width(uint32_t(data.width))
            .height(uint32_t(data.height))
            .levels(1)
            .sampler(Texture::Sampler::SAMPLER_2D)
            .format(Texture::InternalFormat::RGBA8)
            .build(*engine);
    tex->setImage(*engine, 0, std::move(buffer));
    TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);

    // Set up view
    skybox =
      Skybox::Builder().showSun(true).color({165. / 255., 42. / 255., 42. / 255., 1.f}).build(*engine);
    scene->setSkybox(skybox);
    view->setCamera(camera);
    view->setPostProcessingEnabled(false);

    // Create quad renderable
    vb = VertexBuffer::Builder()
           .vertexCount(mesh->vertexBuffer->numVertices)
           .bufferCount(1)
           .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0, 32)
           .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT3, 24, 32)
           .build(*engine);
    vb->setBufferAt(
      *engine, 0,
      VertexBuffer::BufferDescriptor(mesh->vertexBuffer->vertShapes, mesh->vertexBuffer->getSize(), nullptr));
    ib = IndexBuffer::Builder()
           .indexCount(mesh->vertexBuffer->numIndices)
           .bufferType(IndexBuffer::IndexType::USHORT)
           .build(*engine);
    ib->setBuffer(*engine, IndexBuffer::BufferDescriptor(mesh->vertexBuffer->indexShapes,
                                                         mesh->vertexBuffer->getIndexSize(), nullptr));

    Path matPath = getTileMaterialPath();
    std::vector<unsigned char> mat = IOUtil::loadBinaryAsset(matPath.c_str());
    material = Material::Builder().package(mat.data(), mat.size()).build(*engine);
    matInstance = material->createInstance();
    matInstance->setParameter("albedo", tex, sampler);
    matInstance->setParameter("alpha", 1.f);

    renderable = EntityManager::get().create();
    RenderableManager::Builder(1)
      .boundingBox({{-1, -1, -1}, {1, 1, 1}})
      .material(0, matInstance)
      .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb, ib, 0, mesh->vertexBuffer->numIndices)
      .receiveShadows(false)
      .castShadows(false)
      .build(*engine, renderable);

    scene->addEntity(renderable);
    const float aspect = getAspectRatio();
    camera->setProjection(Camera::Projection::ORTHO, -aspect * zoom, aspect * zoom, -zoom, zoom, kNearPlane,
                          kFarPlane);
  }

  virtual void addLight() {
    // Add light sources into the scene.
    utils::EntityManager& em = utils::EntityManager::get();
    light = em.create();
    LightManager::Builder(LightManager::Type::SUN)
      .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
      .intensity(300000)
      .direction({1., 1., 1.})
      .sunAngularRadius(20.f)
      .sunHaloSize(20.f)
      .castShadows(false)
      .castLight(false)
      .build(*engine, light);
    scene->addEntity(light);

    pointLight = em.create();
    LightManager::Builder(LightManager::Type::DIRECTIONAL)
      .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
      .intensity(300000)
      .direction({-1., -1., -1.})
      .sunAngularRadius(20.f)
      .sunHaloSize(20.f)
      .castShadows(false)
      .castLight(false)
      .build(*engine, pointLight);
    scene->addEntity(pointLight);
  }

  virtual void updatePointLight(math::float3 pos) {
    LightManager& lcm = engine->getLightManager();
    const LightManager::Instance& mi = lcm.getInstance(pointLight);
    lcm.setPosition(mi, pos);
    lcm.setDirection(mi, pos);
  }

  virtual void animate(double now) {
    auto& tcm = engine->getTransformManager();
    tcm.setTransform(tcm.getInstance(renderable),
                     filament::math::mat4f::rotation(now, filament::math::float3{0, 0, 1}));
  }

  virtual void shuffle() {
    mesh->shuffle();
    needsDraw = true;
  }

  virtual SwapChain* getSwapChain() {
    return swapChain;
  }

  math::float3 normalizeViewCoord(const math::float2& viewCoord) const {
    math::mat4 projMat = app.camera->getProjectionMatrix();
    math::mat4 invProjMat = app.camera->inverseProjection(projMat);
    float width = float(app.view->getViewport().width);
    float height = float(app.view->getViewport().height);
    math::float4 normalizedView = {viewCoord.x * 2. / width - 1., viewCoord.y * -2. / height + 1., 0., 1.};
    math::float4 clipCoord = invProjMat * normalizedView;
    return {clipCoord.x, clipCoord.y, clipCoord.z};
  }

  std::shared_ptr<Mesh<VB, T>> mesh;

#ifdef USE_SDL
  Logger L;
#endif

  Skybox* skybox;
  Entity renderable;
  Engine* engine = nullptr;
  filament::Renderer* filaRenderer = nullptr;
  SwapChain* swapChain = nullptr;
  Entity cameraEntity;
  Entity light;
  Entity pointLight;
  Camera* camera = nullptr;
  View* view = nullptr;
  Scene* scene = nullptr;

  Texture* tex;
  VertexBuffer* vb;
  IndexBuffer* ib;
  Material* material = nullptr;
  MaterialInstance* matInstance = nullptr;

  Material* anchMaterial = nullptr;

  VertexBuffer* borderVb;
  IndexBuffer* borderIb;
  Entity borderRenderable;
  Material* borderMaterial = nullptr;
  MaterialInstance* borderMatInstance = nullptr;
  Texture* borderTex;

  VertexBuffer* bgVb;
  IndexBuffer* bgIb;
  Entity bgRenderable;
  Material* bgMaterial = nullptr;
  MaterialInstance* bgMatInstance = nullptr;
  Texture* bgTex;

  App app;
  bool needsDraw = false;
  T* dragTile;
  math::float3 lastNormalVec;

  static constexpr double kNearPlane = -1.;
  static constexpr double kFarPlane = 1.;
  float zoom = 1.0f;

  static constexpr std::string_view FILAMAT_FILE_UNLIT = "bakedTextureUnlitTransparent.filamat";
  static constexpr std::string_view FILAMAT_FILE_OPAQUE = "bakedTextureOpaque.filamat";
  static constexpr std::string_view FILAMAT_FILE_LIT = "bakedTextureLitTransparent.filamat";
};

} // namespace tilepuzzles
#endif
