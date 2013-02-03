#ifndef RESOURCEMGR_H
#define RESOURCEMGR_H

#include "utils/singleton.h"
#include "rendering/types.h"

#include <map>
#include <vector>
#include <string>
#include <cassert>

namespace sb
{
	class Image;
	class Mesh;

	class ResourceMgr: public Singleton<ResourceMgr>
	{
	private:
		enum EResourceType {
			ResourceInvalid = 0,
			ResourceTexture,
			ResourceImage,
			ResourceMesh,
			ResourceTerrain,
			ResourceShader,
	
			ResourceCount
		};
	
		std::wstring mBasePath;
		std::vector<std::wstring> mTypePath;
	
		class ResourceRefCounter {
		public:
			ResourceHandle handle;
			unsigned references;
	
			ResourceRefCounter(ResourceHandle h = 0);
			ResourceHandle Attach();
			bool Detach();
		};
	
	
		std::map<EResourceType, std::map<std::wstring, ResourceRefCounter> > mResources;
	
		// default texture, indicating some errors
		TextureId GetDefaultTexture();
	
		bool LoadResource(EResourceType type, const std::wstring& name);
		bool LoadTexture(const std::wstring& name);
		bool LoadMesh(const std::wstring& name);
		bool LoadTerrain(const std::wstring& heightmap);
	
		void DeleteResource(EResourceType type, ResourceHandle& handle);
		ResourceHandle GetResource(EResourceType type, const std::wstring& name);
		bool AddReference(EResourceType type, ResourceHandle handle);
		void FreeResource(EResourceType type, ResourceHandle handle);
	
	public:
		ResourceMgr();
		~ResourceMgr();
		void FreeAllResources();
	
		const std::string GetShaderPath();
	
		// there should be 1 Free* for every Get* call!
		TextureId GetTexture(const std::wstring& name);
		TextureId GetTexture(TextureId id);
		void FreeTexture(TextureId id);
	
		Image* GetImage(const std::wstring& name);
		Image* GetImage(Image* img);
		void FreeImage(Image* img);
	
		Mesh* GetMesh(const std::wstring& name);
		Mesh* GetMesh(Mesh* mesh);
		void FreeMesh(Mesh* mesh);
	
		Mesh* GetTerrain(const std::wstring& heightmap);
		Mesh* GetTerrain(Mesh* terrain);
		void FreeTerrain(Mesh* terrain);
	
		// screw releasing these two, it'll be done at ~ResourceMgr
		Mesh* GetLine();
		Mesh* GetSprite(TextureId tex);
	};
} // namespace sb
	
#define gResourceMgr sb::ResourceMgr::Get()
	
#endif //RESOURCEMGR_H
