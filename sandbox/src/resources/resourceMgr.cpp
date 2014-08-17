#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <IL/il.h>
#include <IL/ilu.h>

#include "resourceMgr.h"

#include "utils/lib.h"
#include "utils/stringUtils.h"
#include "utils/types.h"
#include "utils/logger.h"

#include "mesh.h"
#include "image.h"

namespace sb {

SINGLETON_INSTANCE(ResourceMgr);

std::shared_ptr<Texture> ResourceMgr::getDefaultTexture()
{
    return getTexture("default.png");
}

ResourceMgr::ResourceMgr(const std::string& basePath):
    mBasePath(basePath),
    mTextures(""),
    mImages(mBasePath + "image/"),
    mMeshes(mBasePath + "mesh/"),
    mTerrains(""),
    mVertexShaders(mBasePath + "shader/"),
    mFragmentShaders(mBasePath + "shader/"),
    mGeometryShaders(mBasePath + "shader/"),
    mShaderPrograms()
{
    GLint maxTexSize;
    GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize));
    gLog.trace("max texture size: %d\n", maxTexSize);

    ilInit();
    iluInit();

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

    std::shared_ptr<Mesh> line(new Mesh(Mesh::ShapeLine, lineVertices, {}, {}, {}, lineIndices, {}));
    std::shared_ptr<Mesh> quad(new Mesh(Mesh::ShapeQuad, quadVertices, quadTexcoords, {}, {}, quadIndices, {}));

    mMeshes.addSpecial("line", line);
    mMeshes.addSpecial("quad", quad);
}

void ResourceMgr::freeAll()
{
    mTextures.freeAll();
    mImages.freeAll();
    mMeshes.freeAll();
    mTerrains.freeAll();

    mVertexShaders.freeAll();
    mFragmentShaders.freeAll();
    mGeometryShaders.freeAll();

    gLog.trace("all resources freed\n");
}


std::shared_ptr<Image> ResourceMgr::loadImage(const std::string& name)
{
    std::shared_ptr<Image> img = std::make_shared<Image>();
    if (img->loadFromFile(name)) {
        return img;
    }

    gLog.err("cannot load image %s\n", name.c_str());
    return std::make_shared<Image>();
}

std::shared_ptr<Texture> ResourceMgr::loadTexture(const std::string& name)
{
    gLog.trace("loading texture %s\n", name.c_str());
    return std::make_shared<Texture>(gResourceMgr.getImage(name));
}

std::shared_ptr<Mesh> ResourceMgr::loadMesh(const std::string& name)
{
    gLog.trace("loading mesh %s\n", name.c_str());

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

        std::vector<Vec3> normals;
        if (mesh->HasNormals()) {
            normals.resize(mesh->mNumVertices);
            memcpy(&normals[0], mesh->mNormals, mesh->mNumVertices * sizeof(Vec3));
        }

        std::vector<Vec2> texcoords;
        std::vector<Color> colors;
        std::vector<uint32_t> indices;

        aiString filename;
        std::shared_ptr<Texture> texture;

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
            gLog.err("%s: texture not loaded\n", name.c_str());
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

        return std::shared_ptr<Mesh>(new Mesh(Mesh::ShapeTriangle, vertices, texcoords, {}, normals, indices, texture));
    }

    return {};
}

std::shared_ptr<Mesh> ResourceMgr::loadTerrain(const std::string& heightmap)
{
    gLog.trace("loading terrain %s\n", heightmap.c_str());

    std::shared_ptr<Image> img = gResourceMgr.getImage(heightmap);
    uint32_t w = img->getWidth();
    uint32_t h = img->getHeight();
    uint32_t* data = (uint32_t*)img->getRGBAData();

    gLog.trace("loading terrain %s: %ux%u vertices\n",
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

            sbAssert(idxTimes6 + 5 < numIndices, "not enough terrain indices");

            indices[idxTimes6] = idx;
            indices[idxTimes6 + 1] = indices[idxTimes6 + 4] = idx + w;
            indices[idxTimes6 + 2] = indices[idxTimes6 + 3] = idx + 1;
            indices[idxTimes6 + 5] = idx + w + 1;
        }
    }

    std::vector<Vec3> normals;

    return std::shared_ptr<Mesh>(new Mesh(Mesh::ShapeTriangle, vertices, texcoords, {}, normals, indices, 0));
}

std::shared_ptr<Texture> ResourceMgr::getTexture(const std::string& name)
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

std::vector<std::string> extractAttributes(const std::string& filename)
{
    std::ifstream file(filename);
    std::string line;
    std::vector<std::string> attributes;

    while (std::getline(file, line)) {
        // TODO: ugly
        size_t commentAt = line.find("//");
        if (commentAt != std::string::npos) {
            line = line.substr(0, commentAt);
        }
        commentAt = line.find("/*");
        if (commentAt != std::string::npos) {
            line = line.substr(0, commentAt);
        }

        std::vector<std::string> words = utils::split(line, isspace);
        if (words.size() < 2) {
            continue;
        }

        if (words[0] == "in") {
            if (words[words.size() - 1] == ";") {
                attributes.push_back(words[words.size() - 2]);
            } else {
                std::string& name = words[words.size() - 1];
                attributes.push_back(name.substr(0, name.size() - 1));
            }
        }
    }

    return attributes;
}

std::shared_ptr<Shader> ResourceMgr::getShader(
        const std::string& vertexShaderName,
        const std::string& fragmentShaderName,
        const std::string& geometryShaderName)
{
    bool hasGeometryShader = (geometryShaderName.size() > 0);

    gLog.trace("loading shader: %s, %s, %s\n",
               vertexShaderName.c_str(),
               fragmentShaderName.c_str(),
               hasGeometryShader ? geometryShaderName.c_str()
                                 : "(no geometry shader)");

    auto vertexShader = mVertexShaders.get(vertexShaderName);
    auto fragmentShader = mFragmentShaders.get(fragmentShaderName);
    auto geometryShader = hasGeometryShader
                          ? mGeometryShaders.get(geometryShaderName)
                          : std::shared_ptr<ConcreteShader>();

    if (!vertexShader || !fragmentShader) {
        gLog.err("cannot get shader\n");
        return {};
    }

    ShaderProgramDef programDef { vertexShader,
                                  fragmentShader,
                                  geometryShader ? geometryShader : nullptr };

    auto it = mShaderPrograms.find(programDef);

    if (it != mShaderPrograms.end()) {
        return it->second;
    }

    Shader shader(vertexShader, fragmentShader, geometryShader);
    auto shader_ptr = std::make_shared<Shader>(std::move(shader));
    mShaderPrograms.insert(std::make_pair(programDef, shader_ptr));

    return shader_ptr;
}

std::shared_ptr<Mesh> ResourceMgr::getLine()
{
    return getMesh("*line");
}

std::shared_ptr<Mesh> ResourceMgr::getQuad()
{
    return getMesh("*quad");
}

} // namespace sb
