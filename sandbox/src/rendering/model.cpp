#include "model.h"
#include "../utils/profiler.h"

namespace sb
{
	Model::Model():
		Drawable(ProjectionPerspective)
	{
		PROFILE();
	}
	
	Model::Model(const std::wstring& path):
		Drawable(ProjectionPerspective)
	{
		PROFILE();
	
		mMesh = gResourceMgr.GetMesh(path);
	}
	
	bool Model::LoadFromFile(const std::wstring& path)
	{
		PROFILE();
	
		mMesh = gResourceMgr.GetMesh(path);
		return mMesh != NULL;
	}
}
