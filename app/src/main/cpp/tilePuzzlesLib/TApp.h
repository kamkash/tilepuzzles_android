#ifndef _TAPP_H_
#define _TAPP_H_

#include "ConfigMgr.h"

#ifndef __ANDROID__

#include "GLogger.h"

#endif

#include "Renderer.h"
#include "SliderRenderer.h"

#include "generated/resources/resources.h"
#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>
#include <iostream> // for cerr
#include <utils/EntityManager.h>
#include <utils/Path.h>

using namespace filament;
using utils::Entity;
using utils::EntityManager;
using utils::Path;
using MinFilter = TextureSampler::MinFilter;
using MagFilter = TextureSampler::MagFilter;

namespace tilepuzzles {
struct TApp {

    void init() {
        createRenderer();
        renderer->init();
        mesh = renderer->mesh;
        filamentInit();
    }

    void createRenderer() {
        renderer = std::shared_ptr<Renderer>(new SliderRenderer());
    }

    void filamentInit() {
        Config config;
        config.title = "Tile Puzzles";
        // config.backend = Engine::Backend::VULKAN;

        auto setup = [&](Engine *engine, View *view, Scene *scene) {
            L.info("Using root asset path ", FilamentApp::getRootAssetsPath());
            Path path = FilamentApp::getRootAssetsPath() + "textures/1-30c.png";
            if (!path.exists()) {
                L.error("The texture ", path, " does not exist");
                return -1;
            }
            int w, h, n;
            unsigned char *data = stbi_load(path.c_str(), &w, &h, &n, 4);
            if (data == nullptr) {
                L.error("The texture ", path, " could not be loaded");
                return -1;
            }
            L.info("Loaded texture: y", w, "x", h);
            Texture::PixelBufferDescriptor buffer(
                    data, size_t(w * h * 4), Texture::Format::RGBA, Texture::Type::UBYTE,
                    (Texture::PixelBufferDescriptor::Callback) & stbi_image_free);

            static_assert(sizeof(Vertex) == (4 * 3) + (4 * 3) + (4 * 2),
                          "Strange vertex size.");
            tex = Texture::Builder()
                    .width(uint32_t(w))
                    .height(uint32_t(h))
                    .levels(1)
                    .sampler(Texture::Sampler::SAMPLER_2D)
                    .format(Texture::InternalFormat::RGBA8)
                    .build(*engine);
            tex->setImage(*engine, 0, std::move(buffer));
            TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);

            // Set up view
            skybox = Skybox::Builder().color({0., 0., 0., 0.0}).build(*engine);
            scene->setSkybox(skybox);

            view->setPostProcessingEnabled(false);
            camera = utils::EntityManager::get().create();
            cam = engine->createCamera(camera);
            view->setCamera(cam);

            // Create quad renderable
            vb =
                    VertexBuffer::Builder()
                            .vertexCount(mesh->vertexBuffer->numVertices)
                            .bufferCount(1)
                            .attribute(VertexAttribute::POSITION, 0,
                                       VertexBuffer::AttributeType::FLOAT2, 0,
                                       32)
                            .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2,
                                       24, 32)
                            .build(*engine);
            vb->setBufferAt(*engine, 0,
                            VertexBuffer::BufferDescriptor(mesh->vertexBuffer->quadVertices,
                                                           mesh->vertexBuffer->size, nullptr));
            ib = IndexBuffer::Builder()
                    .indexCount(mesh->vertexBuffer->numIndices)
                    .bufferType(IndexBuffer::IndexType::USHORT)
                    .build(*engine);
            ib->setBuffer(
                    *engine, IndexBuffer::BufferDescriptor(mesh->vertexBuffer->quadIndicies,
                                                           mesh->vertexBuffer->indexSize, nullptr));
            mat = Material::Builder()
                    .package(RESOURCES_BAKEDTEXTURE_DATA, RESOURCES_BAKEDTEXTURE_SIZE)
                    .build(*engine);
            matInstance = mat->createInstance();
            matInstance->setParameter("albedo", tex, sampler);
            renderable = EntityManager::get().create();
            RenderableManager::Builder(1)
                    .boundingBox({{-1, -1, -1},
                                  {1,  1,  1}})
                    .material(0, matInstance)
                    .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb, ib, 0,
                              mesh->vertexBuffer->numIndices)
                    .culling(false)
                    .receiveShadows(false)
                    .castShadows(false)
                    .build(*engine, renderable);
            scene->addEntity(renderable);
            return 0;
        };

        auto cleanup = [&](Engine *engine, View *, Scene *) {
            engine->destroy(skybox);
            engine->destroy(renderable);
            engine->destroy(matInstance);
            engine->destroy(mat);
            engine->destroy(tex);
            engine->destroy(vb);
            engine->destroy(ib);

            engine->destroyCameraComponent(camera);
            utils::EntityManager::get().destroy(camera);
        };

        FilamentApp::get().animate([&](Engine *engine, View *view, double now) {
            const float zoom = 1.5f;
            const uint32_t w = view->getViewport().width;
            const uint32_t h = view->getViewport().height;
            const float aspect = (float) w / h;
            cam->setProjection(Camera::Projection::ORTHO, -aspect * zoom, aspect * zoom, -zoom,
                               zoom, -1, 1);
        });
        FilamentApp::get().run(config, setup, cleanup);
    }

    VertexBuffer *vb;
    IndexBuffer *ib;
    Material *mat;
    MaterialInstance *matInstance;
    Camera *cam;
    Entity camera;
    Skybox *skybox;
    Texture *tex;
    Entity renderable;

    std::shared_ptr <Renderer> renderer;
    std::shared_ptr <Mesh> mesh;

#ifndef __ANDROID__
    constexpr static Logger L = Logger::getLogger();
#endif
};

} // namespace tilepuzzles
#endif