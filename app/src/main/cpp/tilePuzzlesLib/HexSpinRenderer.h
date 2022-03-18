#ifndef _HEX_SPIN_RENDERER_H_
#define _HEX_SPIN_RENDERER_H_

#include "App.h"

#ifdef __ANDROID__

#include "android_debug.h"

#else
#include "GLogger.h"
#endif

#include "HexSpinMesh.h"
#include "TRenderer.h"
#include "Tile.h"

#include <functional>
#include <iostream>
#include <vector>

using namespace filament;
using namespace filament::math;

namespace tilepuzzles {

struct HexSpinRenderer : TRenderer<TriangleVertexBuffer, HexTile> {

    HexSpinRenderer() {
        mesh = std::shared_ptr<Mesh<TriangleVertexBuffer, HexTile>>(new HexSpinMesh());
    }

    virtual void initMesh() {
        mesh->init(CFG);
    }

    virtual HexTile *onRightMouseDown(const float2 &viewCoord) {
        math::float3 clipCoord = normalizeViewCoord(viewCoord);
        HexTile *tile = mesh->hitTest(clipCoord);
        return tile;
    }

    virtual void onMouseMove(const float2 &dragPosition) {
        if (dragTile) {
            math::float3 clipCoord = normalizeViewCoord(dragPosition);
//            HexTile *newTile = mesh->hitTest(clipCoord);
//            if (newTile && !newTile->equals(dragTile)) {
            float2 anchor = std::get<0>(dragAnchor);
            math::float3 anchVec = {dragTile->size.x, 0., 0.};
            math::float3 posVec = geo.translate(clipCoord,
                                                -1. * math::float3(anchor.x, anchor.y, 0.));
            math::float3 pNormal = geo.tcross(anchVec, posVec);

            float angle = 0.F;
            if (abs(pNormal.z) > HexTile::EPS &&
                abs(pNormal.z - lastNormalVec.z) > HexTile::EPS) {
                if (posVec.y > anchVec.y) {
                    if (pNormal.z > lastNormalVec.z) {
                        // normal increase in top-right-left quadrant
                        angle = posVec.x >= 0 ? -ROTATION_ANGLE : ROTATION_ANGLE;
                    } else {
                        // normal decrease in top-right-left quadrant
                        angle = posVec.x >= 0 ? ROTATION_ANGLE : -ROTATION_ANGLE;
                    }
                } else {
                    if (pNormal.z > lastNormalVec.z) {
                        // normal increase in bottom-right-left quadrant
                        angle = posVec.x >= 0 ? -ROTATION_ANGLE : ROTATION_ANGLE;
                    } else {
                        // normal decrease in bottom-right-left quadrant
                        angle = posVec.x >= 0 ? ROTATION_ANGLE : -ROTATION_ANGLE;
                    }
                }
            }
            if (angle != 0.F) {
                rotationAngle += angle;
                mesh->rotateTileGroup(dragAnchor, angle);
                needsDraw = true;
            }
            lastNormalVec = pNormal;
//            }
        }
    }

    virtual HexTile *onMouseDown(const math::float2 &pos) {
        math::float3 clipCoord = normalizeViewCoord(pos);
        dragTile = mesh->hitTest(clipCoord);
        if (dragTile) {
            dragAnchor = mesh->nearestAnchorGroup({clipCoord.x, clipCoord.y});
            auto anchorPoint = std::get<0>(dragAnchor);
            math::float3 anchVec = {dragTile->size.x, 0., 0.};
            math::float3 posVec = geo.translate(clipCoord, -1. * math::float3(anchorPoint.x,
                                                                              anchorPoint.y, 0.));
            math::float3 pNormal = geo.tcross(anchVec, posVec);
            lastNormalVec = pNormal;
        }
        return dragTile;
    }

    virtual HexTile *onMouseUp(const math::float2 &pos) {
        logDragAnchor();
        float angle = snapToAngle();
        if (angle != 0.) {
            mesh->rotateTileGroup(dragAnchor, angle);
            snapToPosition();
            needsDraw = true;
        }
        rotationAngle = 0.f;
        dragTile = nullptr;
        mesh->collectAnchors();
        math::float3 clipCoord = normalizeViewCoord(pos);
        HexTile *tile = mesh->hitTest(clipCoord);
        return tile;
    }

    float snapToAngle() {
        float angle = 0.;
        float angleMultiple = std::abs(rotationAngle / PI_3);
        if (angleMultiple >= 0.5) {
            angleMultiple -= std::floor(angleMultiple);
            angleMultiple = ceil(angleMultiple * 1000.) / 1000.;
            if (rotationAngle >= 0) {
                angle = (angleMultiple >= 0.5) ? PI_3 - angleMultiple : -angleMultiple;
            } else {
                angle = (angleMultiple >= 0.5) ? -(PI_3 - angleMultiple) : angleMultiple;
            }
            angle = ceil(angle * 1000.) / 1000.;
        } else {
            // snap back
            angle = -rotationAngle;
        }
        LOGD("mouseUpAngle: %f", angle);
        return angle;
    }

    void snapToPosition() {
        std::for_each(mesh->tiles.begin(), mesh->tiles.end(), [this](const HexTile &anchTileTarget) {
                        std::for_each(mesh->tiles.begin(), mesh->tiles.end(), [&anchTileTarget](const HexTile &anchTile) {
                          Vertex *targetVerts = *anchTileTarget.triangleVertices;
                          Vertex *srcVerts = *anchTile.iniTriangleVertices;
                          if (abs(targetVerts[0].position.x - srcVerts[0].position.x) <= EPS)
                              targetVerts[0].position.x = srcVerts[0].position.x;
                          if (abs(targetVerts[0].position.y - srcVerts[0].position.y) <= EPS)
                              targetVerts[0].position.y = srcVerts[0].position.y;
                          if (abs(targetVerts[1].position.x - srcVerts[1].position.x) <= EPS)
                              targetVerts[1].position.x = srcVerts[1].position.x;
                          if (abs(targetVerts[1].position.y - srcVerts[1].position.y) <= EPS)
                              targetVerts[1].position.y = srcVerts[1].position.y;
                          if (abs(targetVerts[2].position.x - srcVerts[2].position.x) <= EPS)
                              targetVerts[2].position.x = srcVerts[2].position.x;
                          if (abs(targetVerts[2].position.y - srcVerts[2].position.y) <= EPS)
                              targetVerts[2].position.y = srcVerts[2].position.y;
                        });
                      }
        );
    }


    void logDragAnchor() {
        auto pt = std::get<0>(dragAnchor);
        std::vector<HexTile> tileGroup = std::get<1>(dragAnchor);
        LOGD("anchorPoint: %f %f rotationAngle: %f %f", pt.x, pt.y, rotationAngle,
             rotationAngle / PI_3);
        std::for_each(tileGroup.begin(), tileGroup.end(),
                      [](const HexTile &t) { t.logVertices(); });
    }

    virtual Path getTilesTexturePath() {
#ifndef __ANDROID__
        Path path = FilamentApp::getRootAssetsPath() + "textures/1-30color.png";
#else
        Path path = "textures/1-30color1.png";
#endif
        return path;
    }

    virtual Path getAnchorTexturePath() {
#ifndef __ANDROID__
        Path path = FilamentApp::getRootAssetsPath() + "textures/gear1.png";
#else
        Path path = "textures/gear1.png";
#endif

        return path;
    }

    virtual void draw() {
        TRenderer::draw();
        drawAnchors();
    }

    void drawAnchors() {
        Path path = getAnchorTexturePath();
        int w, h, n;
#ifndef __ANDROID__
        unsigned char* data = ::stbi_load(path.c_str(), &w, &h, &n, 4
#else
        std::vector<unsigned char> res = IOUtil::loadAndroidBinaryAsset(path.c_str());
#endif
        if (res.empty()) {
//      L.error("The texture ", path, " could not be loaded");
            return;
        }
        unsigned char *data = stbi_load_from_memory(res.data(), res.size(), &w, &h, &n, 4);
//    L.info("Loaded texture: y", w, "x", h);
        Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 4), Texture::Format::RGBA,
                                              Texture::Type::UBYTE,
                                              (Texture::PixelBufferDescriptor::Callback) &::stbi_image_free);

        static_assert(sizeof(Vertex) == (4 * 3) + (4 * 3) + (4 * 2), "Strange vertex size.");
        anchTex = Texture::Builder()
            .width(uint32_t(w))
            .height(uint32_t(h))
            .levels(1)
            .sampler(Texture::Sampler::SAMPLER_2D)
            .format(Texture::InternalFormat::RGBA8)
            .build(*engine);
        anchTex->setImage(*engine, 0, std::move(buffer));
        TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);
        // Create quad renderable
        anchVb = VertexBuffer::Builder()
            .vertexCount(mesh->vertexBufferAnchors->numVertices)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0, 32)
            .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT3, 24, 32)
            .build(*engine);
        anchVb->setBufferAt(*engine, 0,
                            VertexBuffer::BufferDescriptor(mesh->vertexBufferAnchors->vertShapes,
                                                           mesh->vertexBufferAnchors->getSize(),
                                                           nullptr));
        anchIb = IndexBuffer::Builder()
            .indexCount(mesh->vertexBufferAnchors->numIndices)
            .bufferType(IndexBuffer::IndexType::USHORT)
            .build(*engine);
        anchIb->setBuffer(*engine,
                          IndexBuffer::BufferDescriptor(mesh->vertexBufferAnchors->indexShapes,
                                                        mesh->vertexBufferAnchors->getIndexSize(),
                                                        nullptr));

#ifndef __ANDROID__
        Path matPath = FilamentApp::getRootAssetsPath() + "textures/bakedTextureLit.filamat";
#else
        Path matPath = "materials/bakedTextureLit.filamat";
#endif
        std::vector<unsigned char> mat = IOUtil::loadAndroidBinaryAsset(matPath.c_str());
//    L.info("bakedTextureLit size", mat.size());
        anchMaterial = Material::Builder().package(mat.data(), mat.size()).build(*engine);

        anchMatInstance = anchMaterial->createInstance();
        anchMatInstance->setParameter("albedo", anchTex, sampler);
        anchMatInstance->setParameter("roughness", 1.f);
        anchMatInstance->setParameter("metallic", 1.f);
        anchMatInstance->setParameter("alpha", .7f);

        anchRenderable = EntityManager::get().create();
        RenderableManager::Builder(1)
            .boundingBox({{-1, -1, -1},
                          {1,  1,  1}})
            .material(0, anchMatInstance)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, anchVb, anchIb, 0,
                      mesh->vertexBufferAnchors->numIndices)
            .culling(false)
            .build(*engine, anchRenderable);
        scene->addEntity(anchRenderable);

        // Add light sources into the scene.
        utils::EntityManager &em = utils::EntityManager::get();
        anchLight = em.create();
        LightManager::Builder(LightManager::Type::SUN)
            .color({.7, .3, .9})
            .intensity(200000)
            .direction({0., 0., -0.6})
            .sunAngularRadius(.5f)
            .castShadows(true)
            .castLight(true)
            .build(*engine, anchLight);
        scene->addEntity(anchLight);
    }

    virtual void destroy() {
        engine->destroy(anchLight);
        engine->destroy(anchRenderable);
        engine->destroy(anchMatInstance);
        engine->destroy(anchTex);
        engine->destroy(anchMaterial);
        engine->destroy(anchVb);
        engine->destroy(anchIb);
        TRenderer::destroy();
    }

    VertexBuffer *anchVb;
    IndexBuffer *anchIb;
    Entity anchRenderable;
    Entity anchLight;
    MaterialInstance *anchMatInstance = nullptr;
    Texture *anchTex;

    std::tuple<math::float2, std::vector<HexTile>> dragAnchor;
    GeoUtil::GeoUtil geo;
    float rotationAngle = 0.;
//    static constexpr float ROTATION_ANGLE = math::F_PI / 3.;
    static constexpr float ROTATION_ANGLE = math::F_PI / 20.;
    static constexpr float PI_3 = math::F_PI / 3.;
    constexpr static float EPS = 0.1F;
    static constexpr const char *CFG = R"({
    "type":"HexSpinner",
      "dimension": {
        "rows": 3,
        "columns": 3
      }
  })";
};
} // namespace tilepuzzles
#endif