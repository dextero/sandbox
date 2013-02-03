// ai* includes should be included first, to avoid new keyword replacement (macro)
#include <assimp.hpp>
#include <aiScene.h>
#include <aiPostProcess.h>

#include <IL/il.h>
#include <IL/ilu.h>

#include "resourceMgr.h"

#include "utils/libUtils.h"
#include "utils/stringUtils.h"
#include "utils/types.h"
#include "utils/logger.h"
#include "utils/profiler.h"

#include "mesh.h"
#include "image.h"

namespace sb
{
	SINGLETON_INSTANCE(ResourceMgr);
	
	
	ResourceMgr::ResourceRefCounter::ResourceRefCounter(ResourceHandle h):
		handle(h),
		references(0)
	{
		PROFILE();
	}
	
	ResourceHandle ResourceMgr::ResourceRefCounter::Attach()
	{
		PROFILE();
	
		++references;
		return handle;
	}
	
	bool ResourceMgr::ResourceRefCounter::Detach()
	{
		PROFILE();
	
		return --references == 0;
	}
	
	TextureId ResourceMgr::GetDefaultTexture()
	{
		PROFILE();
	
		return GetTexture(L"default.png");
	}
	
	ResourceMgr::ResourceMgr():
		mBasePath(L"data/")
	{
		PROFILE();
	
		GLint maxTexSize;
		GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize));
		gLog.Info("max texture size: %d\n", maxTexSize);
	
		mTypePath.resize(ResourceCount);
		mTypePath[ResourceTexture] = L"texture/";
		mTypePath[ResourceMesh] = L"mesh/";
		mTypePath[ResourceImage] = L"image/";
		mTypePath[ResourceShader] = L"shader/";
	
		ilInit();
	
		// init meshs' vertex buffer
		Mesh::msBuffer = new SharedVertexBuffer(SharedVertexBuffer::BufferTexcoord);
	
		// released in destructor
		Mesh* line = new Mesh();
		Mesh* quad = new Mesh();
	
		Vec3 lineVertices[] = {
			Vec3(0.f, 0.f, 0.f),
			Vec3(1.f, 1.f, 1.f)
		};
	
		uint lineIndices[] = { 0, 1 };
	
		Vec3 quadVertices[] = {
			Vec3(-1.f, -1.f, 0.f),
			Vec3(1.f, -1.f, 0.f),
			Vec3(1.f, 1.f, 0.f),
			Vec3(-1.f, 1.f, 0.f)
		};
	
		Vec2 quadTexcoords[] = {
			Vec2(0.f, 1.f),
			Vec2(0.f, 0.f),
			Vec2(1.f, 0.f),
			Vec2(1.f, 1.f)
		};
	
		uint quadIndices[] = { 0, 1, 2, 3 };
	
		line->Create(Mesh::ShapeLine, lineVertices, NULL, NULL, 2, lineIndices, 2, 0);
		quad->Create(Mesh::ShapeQuad, quadVertices, quadTexcoords, NULL, 4, quadIndices, 4, GetDefaultTexture());
	
		mResources[ResourceMesh].insert(std::make_pair(L"*line", ResourceRefCounter((ResourceHandle)line)));
		mResources[ResourceMesh].insert(std::make_pair(L"*quad", ResourceRefCounter((ResourceHandle)quad)));
	}
	
	ResourceMgr::~ResourceMgr()
	{
		PROFILE();
	
		FreeAllResources();
	
		// releasing meshs' vertex buffer
		SAFE_RELEASE(Mesh::msBuffer);
	}
	
	void ResourceMgr::FreeAllResources()
	{
		for (std::map<EResourceType, std::map<std::wstring, ResourceRefCounter> >::iterator it = mResources.begin(); it != mResources.end(); ++it)
		{
			std::map<std::wstring, ResourceRefCounter>& res = it->second;
			for (std::map<std::wstring, ResourceRefCounter>::iterator it2 = res.begin(); it2 != res.end(); ++it2)
				DeleteResource(it->first, it2->second.handle);
		}
		mResources.clear();
	
		gLog.Info("all resources freed\n");
	}
	
	
	bool ResourceMgr::LoadResource(EResourceType type, const std::wstring& name)
	{
		PROFILE();
	
		switch (type)
		{
		case ResourceTexture:
			return LoadTexture(name);
		case ResourceImage:
			{
				gLog.Info("loading image %ls\n", name.c_str());
	
				Image* img = new Image();
				mResources[ResourceImage].insert(std::make_pair(name, ResourceRefCounter((ResourceHandle)img)));
				return img->LoadFromFile(mBasePath + mTypePath[ResourceImage] + name);
			}
		case ResourceMesh:
			return LoadMesh(name);
		case ResourceTerrain:
			return LoadTerrain(name);
		default:
			gLog.Warn("invalid resource type: %d\n", (int)type);
			assert(!"Invalid resource type.");
		}
		return false;
	}
	
	bool ResourceMgr::LoadTexture(const std::wstring& name)
	{
		PROFILE();
		gLog.Info("loading texture %ls\n", name.c_str());
	
		std::map<std::wstring, ResourceRefCounter>& textures = mResources[ResourceTexture];
	
		if (textures.find(name) != textures.end())
		{
			gLog.Warn("loading an already loaded texture, wtf\n");
			return true;
		}
	
		ILuint image = ilGenImage();
		IL_CHECK_RET(ilBindImage(image), false);

#ifdef PLATFORM_WIN32
		IL_CHECK_RET(ilLoadImage((mBasePath + mTypePath[ResourceTexture] + name).c_str()), false);
#else //PLATFORM_LINUX
		IL_CHECK_RET(ilLoadImage(StringUtils::ToString(mBasePath + mTypePath[ResourceTexture] + name).c_str()), false);
#endif // PLATFORM_WIN32
		
		uint maxTexSize;
		GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&maxTexSize));
	
		uint imgWidth = ilGetInteger(IL_IMAGE_WIDTH), imgHeight = ilGetInteger(IL_IMAGE_HEIGHT), potWidth = 1, potHeight = 1;
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
		GL_CHECK_RET(glGenTextures(1, &texture), false);
		GL_CHECK_RET(glBindTexture(GL_TEXTURE_2D, texture), false);
		GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	
		GL_CHECK_RET(glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), potWidth, potHeight, 0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData()), false);
	
		// generate mipmaps
		GL_CHECK_RET(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT), false);
		GL_CHECK_RET(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT), false);
		GL_CHECK_RET(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR), false);
		GL_CHECK_RET(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR), false);
		GL_CHECK_RET(glGenerateMipmap(GL_TEXTURE_2D), false);
	
		// from this line on, a function failure does not mean that texture has not been loaded
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, prevTex));
		// restore previous image
		IL_CHECK(ilDeleteImage(image));
	
		textures.insert(std::make_pair(name, ResourceRefCounter((ResourceHandle)texture)));
	
		return true;
	}
	
	bool ResourceMgr::LoadMesh(const std::wstring& name)
	{
		PROFILE();
		gLog.Info("loading mesh %ls\n", name.c_str());
	
		std::map<std::wstring, ResourceRefCounter>& meshs = mResources[ResourceMesh];
	
		// TODO: wiele tekstur
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(StringUtils::ToString(mBasePath + mTypePath[ResourceMesh] + name),
			aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
	
		if (!scene)
			return false;
	
		if (!scene->HasMeshes())
			return false;
	
		for (uint i = 0; i < scene->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[i];
	
			if (!mesh->HasPositions())
				continue;
	
			if (!mesh->HasTextureCoords(0))
				continue;
	
			// texcoords
			Vec2* texcoords = NULL;
			aiString filename;
			uint texture;
	
			aiReturn result = scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &filename);
			if (result == AI_SUCCESS)
			{
				texture = GetTexture(StringUtils::ToWString(filename.data));
	
				texcoords = new Vec2[mesh->mNumVertices];
				for (uint i = 0; i < mesh->mNumVertices; ++i)
					texcoords[i] = Vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			}
			else
				gLog.Err("%ls: texture not loaded\n", name.c_str());
	
			// indices
			uint numIndices = 0;
			for (uint i = 0; i < mesh->mNumFaces; ++i)
				numIndices += mesh->mFaces[i].mNumIndices;
	
			uint* indices = new uint[numIndices];
			numIndices = 0;
			for (uint i = 0; i < mesh->mNumFaces; ++i)
			{
				memcpy(indices + numIndices, mesh->mFaces[i].mIndices, mesh->mFaces[i].mNumIndices * sizeof(uint));
				numIndices += mesh->mFaces[i].mNumIndices;
			}
	
			// mesh
			Mesh* terrainMesh = new Mesh();
			if (!terrainMesh->Create(Mesh::ShapeTriangle, (Vec3*)mesh->mVertices, texcoords, NULL, mesh->mNumVertices, indices, numIndices, texture))
				return false;
	
			delete[] texcoords;
			delete[] indices;
	
			meshs.insert(std::make_pair(name, ResourceRefCounter((ResourceHandle)terrainMesh)));
		}
		
		return true;
	}
	
	bool ResourceMgr::LoadTerrain(const std::wstring& heightmap)
	{
		PROFILE();
		gLog.Info("loading terrain %ls\n", heightmap.c_str());
	
		std::map<std::wstring, ResourceRefCounter>& meshs = mResources[ResourceTerrain];
	
		/*
		FILE* f = _wfopen(heightmap.c_str(), L"r");
		fseek(f, 0, SEEK_END);
		uint filesize = ftell(f);
		fseek(f, 0, SEEK_SET);
		short* data = new short[filesize / 2];
		fread(data, 1, filesize, f);
		fclose(f);
		uint w = (uint)sqrt((double)(filesize / 2)),
			 h = w;
	
		Vec3* vertices = new Vec3[w * h];
		for (uint i = 0; i < w * h; ++i)
			vertices[i] = Vec3((float)(i % w), (float)data[i], (float)(i / w));
		*/
	
		Image& img = *GetImage(heightmap);
		uint w = img.GetWidth(),
			h = img.GetHeight();
		unsigned int* data = (unsigned int*)img.GetData();
	
		gLog.Info("loading terrain %ls: %ux%u vertices\n", heightmap.c_str(), w, h);
	
		Vec3* vertices = new Vec3[w * h];
		for (uint i = 0; i < w * h; ++i)
			vertices[i] = Vec3((float)(i % w), (float)((data[i] & 0x00FF0000) | ((data[i] & 0xFF000000) << 16) | ((data[i] & 0x0000FF00) >> 16)) / 100000.f, (float)(i / w));
	
		Vec2* texcoords = new Vec2[w * h];
	
		// for now, let's use tiling only
		/*switch (texturingMode)
		{
		case TexturingStretch:
			for (uint i = 0; i < w * h; ++i)
				texcoords[i] = Vec2((float)(i % w) / (float)w, (float)(i / w) / (float)w);
			break;
		case TexturingTile:*/
			for (uint i = 0; i < w * h; ++i)
				texcoords[i] = Vec2((i % w) % 2 ? 0.f : 1.f, (i / w) % 2 ? 0.f : 1.f);
			/*break;
		default:
			// wtf
			fprintf(stderr, "Terrain::Terrain: invalid texturing mode\n");
			SAFE_RELEASE(texcoords);
			break;
		}*/
	
		uint numIndices = (w - 1) * (h - 1) * 6;
		uint* indices = new uint[numIndices];
		for (uint x = 0; x < (w - 2); ++x)
			for (uint y = 0; y < (h - 1); ++y)
			{
				uint idx = x * h + y;
				uint idxTimes6 = idx * 6;
	
				assert(idxTimes6 + 5 < numIndices);
	
				indices[idxTimes6] = idx;
				indices[idxTimes6 + 1] = indices[idxTimes6 + 4] = idx + w;
				indices[idxTimes6 + 2] = indices[idxTimes6 + 3] = idx + 1;
				indices[idxTimes6 + 5] = idx + w + 1;
			}
	
		// mesh
		Mesh* terrain = new Mesh();
		bool ret = terrain->Create(Mesh::ShapeTriangle, vertices, texcoords, NULL, w * h, indices, numIndices, 0);
	
		delete[] vertices;
		delete[] texcoords;
		delete[] indices;
	
		meshs.insert(std::make_pair(heightmap, ResourceRefCounter((ResourceHandle)terrain)));
	
		return ret;
	}
	
	void ResourceMgr::DeleteResource(EResourceType type, ResourceHandle& handle)
	{
		PROFILE();
	
		switch (type)
		{
		case ResourceTexture:
			{
				// for compatibility in 64bit builds
				GLuint texture = (GLuint)handle;
				glDeleteTextures(1, &texture);
				break;
			}
		case ResourceImage:
			delete (Image*)handle;
			break;
		case ResourceMesh:
		case ResourceTerrain:
			delete (Mesh*)handle;
			break;
		default:
			gLog.Warn("invalid resource type: %d\n", (int)type);
			assert(!"Invalid resource type.");
		}
	}
	
	ResourceHandle ResourceMgr::GetResource(EResourceType type, const std::wstring& name)
	{
		PROFILE();
	
		std::map<std::wstring, ResourceRefCounter>& resources = mResources[type];
	
		if (resources.find(name) != resources.end())
			return resources[name].Attach();
		else if (LoadResource(type, name))
			return resources[name].Attach();
		else
		{
			gLog.Err("couldn't load resource %ls\n", name.c_str());
			return (ResourceHandle)0;
		}
	}
	
	bool ResourceMgr::AddReference(EResourceType type, ResourceHandle handle)
	{
		PROFILE();
	
		std::map<std::wstring, ResourceRefCounter>& resources = mResources[type];
	
		for (std::map<std::wstring, ResourceRefCounter>::iterator it = resources.begin(); it != resources.end(); ++it)
			if (it->second.handle == handle)
			{
				it->second.Attach();
				return true;
			}
	
		return false;
	}
	
	void ResourceMgr::FreeResource(EResourceType type, ResourceHandle handle)
	{
		PROFILE();
	
		std::map<std::wstring, ResourceRefCounter>& resources = mResources[type];
	
		for (std::map<std::wstring, ResourceRefCounter>::iterator it = resources.begin(); it != resources.end(); ++it)
			if (it->second.handle == handle)
			{
				if (it->second.Detach())
				{
					DeleteResource(type, it->second.handle);
					resources.erase(it);
				}
	
				break;
			}
	}
	
	
	const std::string ResourceMgr::GetShaderPath()
	{
		PROFILE();
	
		return StringUtils::ToString(mBasePath + mTypePath[ResourceShader]);
	}
	
	// there should be 1 Free* for every Get* call!
	TextureId ResourceMgr::GetTexture(const std::wstring& name)
	{
		PROFILE();
	
		return (TextureId)GetResource(ResourceTexture, name);
	}
	
	TextureId ResourceMgr::GetTexture(TextureId id)
	{
		PROFILE();
	
		return AddReference(ResourceTexture, (ResourceHandle)id) ? id : 0;
	}
	
	void ResourceMgr::FreeTexture(TextureId id)
	{
		PROFILE();
	
		FreeResource(ResourceTexture, (ResourceHandle)id);
	}
	
	Image* ResourceMgr::GetImage(const std::wstring& name)
	{
		PROFILE();
	
		return (Image*)GetResource(ResourceImage, name);
	}
	
	Image* ResourceMgr::GetImage(Image* img)
	{
		PROFILE();
	
		return AddReference(ResourceImage, (ResourceHandle)img) ? img : NULL;
	}
	
	void ResourceMgr::FreeImage(Image* img)
	{
		PROFILE();
	
		FreeResource(ResourceImage, (ResourceHandle)img);
	}
	
	Mesh* ResourceMgr::GetMesh(const std::wstring& name)
	{
		PROFILE();
	
		return (Mesh*)GetResource(ResourceMesh, name);
	}
	
	Mesh* ResourceMgr::GetMesh(Mesh* mesh)
	{
		PROFILE();
	
		return AddReference(ResourceMesh, (ResourceHandle)mesh) ? mesh : NULL;
	}
	
	void ResourceMgr::FreeMesh(Mesh* mesh)
	{
		PROFILE();
	
		FreeResource(ResourceMesh, (ResourceHandle)mesh);
	}
	
	Mesh* ResourceMgr::GetTerrain(const std::wstring& heightmap)
	{
		PROFILE();
	
		return (Mesh*)GetResource(ResourceTerrain, heightmap);
	}
	
	Mesh* ResourceMgr::GetTerrain(Mesh* terrain)
	{
		PROFILE();
	
		return AddReference(ResourceTerrain, (ResourceHandle)terrain) ? terrain : NULL;
	}
	
	void ResourceMgr::FreeTerrain(Mesh* terrain)
	{
		PROFILE();
	
		FreeResource(ResourceTerrain, (ResourceHandle)terrain);
	}
	
	Mesh* ResourceMgr::GetLine()
	{
		PROFILE();
	
		return GetMesh(L"*line");
	}
	
	Mesh* ResourceMgr::GetSprite(TextureId texture)
	{
		PROFILE();
	
		Mesh* ret = GetMesh(L"*quad");
		if (ret)
			ret->mTexture = texture;
		return ret;
	}
} // namespace sb
