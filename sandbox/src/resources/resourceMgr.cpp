#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <IL/il.h>
#include <IL/ilu.h>

#include "resourceMgr.h"

#include "utils/libUtils.h"
#include "utils/stringUtils.h"
#include "utils/types.h"
#include "utils/logger.h"

#include "mesh.h"
#include "image.h"

namespace sb
{
    SINGLETON_INSTANCE(ResourceMgr);

    std::shared_ptr<TextureId> ResourceMgr::getDefaultTexture()
    {
        return getTexture("default.png");
    }

    ResourceMgr::ResourceMgr(const std::string& basePath):
        mBasePath(basePath),
        mShaderPath(mBasePath + "shader/"),
        mTextures(mBasePath + "texture/"),
        mImages(mBasePath + "image/"),
        mMeshes(mBasePath + "mesh/"),
        mTerrains(mBasePath + "image/")
    {
        GLint maxTexSize;
        GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize));
        gLog.Info("max texture size: %d\n", maxTexSize);

        ilInit();

        // init meshs' vertex buffer
        Mesh::msBuffer = new SharedVertexBuffer(SharedVertexBuffer::BufferTexcoord);

        // released in destructor
        std::shared_ptr<Mesh> line = std::make_shared<Mesh>();
        std::shared_ptr<Mesh> quad = std::make_shared<Mesh>();

        std::vector<Vec3> lineVertices {
            { 0.f, 0.f, 0.f },
            { 1.f, 1.f, 1.f }
        };

        std::vector<uint32_t> lineIndices { 0, 1 };

        std::vector<Vec3> quadVertices {
            { -1.f, -1.f, 0.f },
            {  1.f, -1.f, 0.f },
            {  1.f,  1.f, 0.f },
            { -1.f,  1.f, 0.f }
        };

        std::vector<Vec2> quadTexcoords {
            { 0.f, 1.f },
            { 0.f, 0.f },
            { 1.f, 0.f },
            { 1.f, 1.f }
        };

        std::vector<uint32_t> quadIndices { 0, 1, 2, 3 };

        line->Create(Mesh::ShapeLine, lineVertices, {}, {}, lineIndices, 0);
        quad->Create(Mesh::ShapeQuad, quadVertices, quadTexcoords, {}, quadIndices, getDefaultTexture());

        mMeshes.addSpecial("line", line);
        mMeshes.addSpecial("quad", quad);
    }

    ResourceMgr::~ResourceMgr()
    {
        // releasing meshs' vertex buffer
        SAFE_RELEASE(Mesh::msBuffer);
    }

    void ResourceMgr::freeAll()
    {
        mTextures.freeAll();
        mImages.freeAll();
        mMeshes.freeAll();
        mTerrains.freeAll();

        gLog.Info("all resources freed\n");
    }


    std::shared_ptr<Image> ResourceMgr::loadImage(const std::string& name)
    {
        std::shared_ptr<Image> img = std::make_shared<Image>();
        if (img->LoadFromFile(name)) {
            return img;
        }

        gLog.Err("cannot load image %s\n", name.c_str());
        return std::make_shared<Image>();
    }

    std::shared_ptr<TextureId> ResourceMgr::loadTexture(const std::string& name)
    {
        gLog.Info("loading texture %s\n", name.c_str());

        ILuint image = ilGenImage();
        IL_CHECK_RET(ilBindImage(image), false);

#ifdef PLATFORM_WIN32
        IL_CHECK_RET(ilLoadImage(StringUtils::toWString(name).c_str()), false);
#else //PLATFORM_LINUX
        IL_CHECK_RET(ilLoadImage(name.c_str()), false);
#endif // PLATFORM_WIN32

        uint32_t maxTexSize;
        GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&maxTexSize));

        uint32_t imgWidth = ilGetInteger(IL_IMAGE_WIDTH);
        uint32_t imgHeight = ilGetInteger(IL_IMAGE_HEIGHT);
        uint32_t potWidth = 1;
        uint32_t potHeight = 1;

        while (potWidth < imgWidth && potWidth < maxTexSize) potWidth *= 2;
        while (potHeight < imgHeight && potHeight < maxTexSize) potHeight *= 2;

        if (potWidth != imgWidth || potHeight != imgHeight)
        {
            gLog.Info("scaling texture: %ux%u to %ux%u\n", imgWidth, imgHeight, potWidth, potHeight);
            iluScale(potWidth, potHeight, 1);
        }

        GLuint texture;
        GLuint prevTex;
        // save current image
        GL_CHECK(glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&prevTex));

        // copy image to opengl
        GL_CHECK_RET(glGenTextures(1, &texture), {});
        GL_CHECK_RET(glBindTexture(GL_TEXTURE_2D, texture), {});
        GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

        GL_CHECK_RET(glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), potWidth, potHeight, 0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData()), {});

        // generate mipmaps
        GL_CHECK_RET(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT), {});
        GL_CHECK_RET(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT), {});
        GL_CHECK_RET(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR), {});
        GL_CHECK_RET(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR), {});
        GL_CHECK_RET(glGenerateMipmap(GL_TEXTURE_2D), {});

        // from this line on, a function failure does not mean that texture has not been loaded
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, prevTex));
        // restore previous image
        IL_CHECK(ilDeleteImage(image));

        return std::make_shared<TextureId>(texture);
    }

    std::shared_ptr<Mesh> ResourceMgr::loadMesh(const std::string& name)
    {
        gLog.Info("loading mesh %s\n", name.c_str());

        // TODO: wiele tekstur
        Assimp::Importer importer;
        const uint32_t importerFlags = aiProcess_CalcTangentSpace
                                       | aiProcess_Triangulate
                                       | aiProcess_JoinIdenticalVertices
                                       | aiProcess_SortByPType;
        const aiScene* scene = importer.ReadFile(name, importerFlags);

        if (!scene || !scene->HasMeshes()) {
            return {};
        }

        for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[i];

            if (!mesh->HasPositions()
                    || !mesh->HasTextureCoords(0)) {
                continue;
            }

            // texcoords
            std::vector<Vec3> vertices(mesh->mNumVertices);
            memcpy(&vertices[0], mesh->mVertices, mesh->mNumVertices * sizeof(Vec3));

            std::vector<Vec2> texcoords;
            std::vector<Color> colors;
            std::vector<uint32_t> indices;

            aiString filename;
            std::shared_ptr<TextureId> texture;

            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiReturn result = material->GetTexture(aiTextureType_DIFFUSE,
                                                   0, &filename);
            if (result == AI_SUCCESS) {
                texture = gResourceMgr.getTexture(filename.data);

                texcoords.reserve(mesh->mNumVertices);
                for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
                    texcoords.emplace_back(Vec2(mesh->mTextureCoords[0][i].x,
                                                mesh->mTextureCoords[0][i].y));
                }
            } else {
                gLog.Err("%s: texture not loaded\n", name.c_str());
            }

            // indices
            uint32_t numIndices = 0;
            for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
                numIndices += mesh->mFaces[i].mNumIndices;

            indices.resize(numIndices);
            numIndices = 0;
            for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
                memcpy(&indices[numIndices],
                       mesh->mFaces[i].mIndices,
                       mesh->mFaces[i].mNumIndices * sizeof(uint32_t));
                numIndices += mesh->mFaces[i].mNumIndices;
            }

            // mesh
            std::shared_ptr<Mesh> loadedMesh = std::make_shared<Mesh>();
            if (!loadedMesh->Create(Mesh::ShapeTriangle,
                                    vertices, texcoords,
                                    {}, indices, texture)) {
                return {};
            }

            return loadedMesh;
        }

        return {};
    }

    std::shared_ptr<Mesh> ResourceMgr::loadTerrain(const std::string& heightmap)
    {
        gLog.Info("loading terrain %s\n", heightmap.c_str());

        std::shared_ptr<Image> img = gResourceMgr.getImage(heightmap);
        uint32_t w = img->GetWidth();
        uint32_t h = img->GetHeight();
        uint32_t* data = (uint32_t*)img->GetData();

        gLog.Info("loading terrain %s: %ux%u vertices\n",
                  heightmap.c_str(), w, h);

#define RGBA_TO_HEIGHT(rgba) \
    ((float)(((rgba) & 0x00ff0000) \
           | (((rgba) & 0xff000000) << 8) \
           | (((rgba) & 0x0000ff00) << 16)) \
        / 100000.0f)

        std::vector<Vec3> vertices;
        for (uint32_t i = 0; i < w * h; ++i) {
            vertices.emplace_back(Vec3((float)(i % w),
                                        RGBA_TO_HEIGHT(data[i]),
                                        (float)(i / w)));
        }

        std::vector<Vec2> texcoords;

        // for now, let's use tiling only
        /*switch (texturingMode)
        {
        case TexturingStretch:
            for (uint32_t i = 0; i < w * h; ++i)
                texcoords[i] = Vec2((float)(i % w) / (float)w, (float)(i / w) / (float)w);
            break;
        case TexturingTile:*/
            for (uint32_t i = 0; i < w * h; ++i) {
                texcoords.emplace_back(Vec2((i % w) % 2 ? 0.f : 1.f,
                                            (i / w) % 2 ? 0.f : 1.f));
            }
            /*break;
        default:
            // wtf
            fprintf(stderr, "Terrain::Terrain: invalid texturing mode\n");
            SAFE_RELEASE(texcoords);
            break;
        }*/

        uint32_t numIndices = (w - 1) * (h - 1) * 6;
        std::vector<uint32_t> indices(numIndices);

        for (uint32_t x = 0; x < (w - 2); ++x) {
            for (uint32_t y = 0; y < (h - 1); ++y) {
                uint32_t idx = x * h + y;
                uint32_t idxTimes6 = idx * 6;

                assert(idxTimes6 + 5 < numIndices);

                indices[idxTimes6] = idx;
                indices[idxTimes6 + 1] = indices[idxTimes6 + 4] = idx + w;
                indices[idxTimes6 + 2] = indices[idxTimes6 + 3] = idx + 1;
                indices[idxTimes6 + 5] = idx + w + 1;
            }
        }

        // mesh
        std::shared_ptr<Mesh> terrain = std::make_shared<Mesh>();
        if (terrain->Create(Mesh::ShapeTriangle,
                            vertices, texcoords, {}, indices, 0)) {
            return terrain;
        }

        return {};
    }

    void ResourceMgr::freeTexture(const std::shared_ptr<TextureId>& texture)
    {
        glDeleteTextures(1, texture.get());
    }

    std::shared_ptr<TextureId> ResourceMgr::getTexture(const std::string& name)
    {
        return mTextures.get(name);
    }

    std::shared_ptr<Image> ResourceMgr::getImage(const std::string& name)
    {
        return mImages.get(name);
    }

    std::shared_ptr<Mesh> ResourceMgr::getMesh(const std::string& name)
    {
        return mMeshes.get(name);
    }

    std::shared_ptr<Mesh> ResourceMgr::getTerrain(const std::string& heightmap)
    {
        return mTerrains.get(heightmap);
    }

    std::shared_ptr<Mesh> ResourceMgr::getLine()
    {
        return getMesh("*line");
    }

    std::shared_ptr<Mesh> ResourceMgr::getSprite(const std::shared_ptr<TextureId>& texture)
    {
        std::shared_ptr<Mesh> mesh = getMesh("*quad");
        if (!mesh) {
            return {};
        }

        mesh->SetTexture(texture);
        return mesh;
   }
} // namespace sb
