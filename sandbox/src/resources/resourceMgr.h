#ifndef RESOURCEMGR_H
#define RESOURCEMGR_H

#include "rendering/types.h"
#include "utils/logger.h"
#include "utils/singleton.h"

#include <map>
#include <vector>
#include <string>
#include <cassert>
#include <memory>

namespace sb
{
    class Image;
    class Mesh;

    template<typename T>
    void noop(const std::shared_ptr<T>&) {}

    template<
        typename T,
        std::shared_ptr<T>(*LoadFunc)(const std::string&),
        void(*ReleaseFunc)(const std::shared_ptr<T>&) = noop<T>
    >
    class SpecificResourceMgr
    {
    public:
        SpecificResourceMgr(const std::string& basePath):
            mBasePath(basePath)
        {
            if (mBasePath[mBasePath.size() - 1] != '/') { 
                mBasePath += "/";
            }
        }

        std::string getBasePath() const
        {
            return mBasePath();
        }

        std::shared_ptr<T> get(const std::string& name)
        {
            auto it = mResources.find(name);
            if (it != mResources.end()) {
                return it->second;
            }

            std::shared_ptr<T> resource = LoadFunc(name);
            if (resource) {
                mResources.insert(std::make_pair(name, resource));
            }

            return resource;
        }

        static bool isSpecial(const std::string& resourceName)
        {
            return resourceName.size() > 0
                    && resourceName[0] == '*';
        }

        static std::string makeSpecial(const std::string& name)
        {
            return "*" + name;
        }

        void addSpecial(const std::string& name,
                        const std::shared_ptr<T>& resource)
        {
            mResources.insert(std::make_pair(makeSpecial(name), resource));
        }

        std::shared_ptr<T> getSpecial(const std::string& name) const
        {
            auto it = mResources.find(makeSpecial(name));
            if (it != mResources.end()) {
                return it->second;
            }

            return {};
        }

        void freeUnused()
        {
            for (auto it = mResources.begin(); it != mResources.end();) {
                if (it->second.use_count() == 1
                        && !isSpecial(it->first)) {
                    gLog.Info("ResourceMgr: removing %s\n", (mBasePath + it->first).c_str());
                    ReleaseFunc(it->second);
                    it = mResources.erase(it);
                } else {
                    ++it;
                }
            }
        }

        void freeAll()
        {
            mResources.clear();
        }

    private:
        std::string mBasePath;
        std::map<std::string, std::shared_ptr<T>> mResources;
    };

    class ResourceMgr: public Singleton<ResourceMgr>
    {
    public:
        ResourceMgr(const std::string& basePath = "data/");
        ~ResourceMgr();

        void freeUnused();
        void freeAll();

        const std::string getShaderPath() { return mShaderPath; }

        std::shared_ptr<TextureId> getTexture(const std::string& name);
        std::shared_ptr<Image> getImage(const std::string& name);
        std::shared_ptr<Mesh> getMesh(const std::string& name);
        std::shared_ptr<Mesh> getTerrain(const std::string& heightmap);

        std::shared_ptr<Mesh> getLine();
        std::shared_ptr<Mesh> getSprite(const std::shared_ptr<TextureId>& tex);

    private:
        static std::shared_ptr<TextureId> loadTexture(const std::string& path);
        static std::shared_ptr<Image> loadImage(const std::string& path);
        static std::shared_ptr<Mesh> loadMesh(const std::string& path);
        static std::shared_ptr<Mesh> loadTerrain(const std::string& heightmapPath);

        static void freeTexture(const std::shared_ptr<TextureId>& texture);

        const std::string mBasePath;
        const std::string mShaderPath;

        SpecificResourceMgr<
            TextureId,
            &ResourceMgr::loadTexture,
            &ResourceMgr::freeTexture
        > mTextures;
        SpecificResourceMgr<
            Image,
            &ResourceMgr::loadImage
        > mImages;
        SpecificResourceMgr<
            Mesh,
            &ResourceMgr::loadMesh
        > mMeshes;
        SpecificResourceMgr<
            Mesh,
            &ResourceMgr::loadTerrain
        > mTerrains;

        // default texture, indicating some errors
        std::shared_ptr<TextureId> getDefaultTexture();
    };
} // namespace sb

#define gResourceMgr sb::ResourceMgr::Get()

#endif //RESOURCEMGR_H
