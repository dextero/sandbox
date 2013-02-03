#ifndef RENDERER_H
#define RENDERER_H

#ifdef PLATFORM_WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#	undef near
#	undef far
#endif // PLATFORM_WIN32

#include <cassert>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "rendering/color.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "rendering/drawable.h"
#include "utils/allocator.h"

#include <vector>

namespace sb
{
	class Renderer: OWN_ALLOC
	{
	private:
		HGLRC mGLContext;
		std::vector<Drawable> mDrawablesBuffer;
		bool mUseDrawableBuffering;
	
		bool InitGLEW();
	
		enum EFilterType {
			FilterShader,
			FilterTexture,
			FilterDepth,
			FilterProjection,
			FilterShaderTextureProjectionDepth
		};
		void FilterDrawables(EFilterType filter);
	
	public:
		Camera mCamera;
	
		Renderer();
		~Renderer();
	
		bool Init(HWND wnd);
		void SetClearColor(const Color& c);
		void Clear();
		void SetViewport(unsigned x, unsigned y, unsigned cx, unsigned cy);
	
		void Draw(Drawable& d);
		void DrawAll();
	
		enum EFeature {
			FeatureBackfaceCulling = RENDERER_BACKFACE_CULLING,
			FeatureDepthTest = RENDERER_DEPTH_TEST,
			FeatureAlphaBlending = RENDERER_ALPHA_BLENDING
		};
	
		void EnableFeature(EFeature feature, bool enable = true);
		void SaveScreenshot(const wchar_t* filename, int width, int height);
	};
} // namespace sb

#endif //RENDERER_H