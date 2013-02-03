#ifndef NO_CHECK_MACROS
#	define NO_CHECK_MACROS
#endif

#include "libUtils.h"
#include "logger.h"

#ifdef PLATFORM_WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#endif // PLATFORM_WIN32

#include <GL/glu.h>
#include <IL/ilu.h>

namespace sb
{
	namespace Utils
	{
		// returns true on error
		bool GLCheck(const char* file, int line, const char* call)
		{
			GLuint err = glGetError();
			if (err != GL_NO_ERROR)
				gLog.Err("GL error: \"%s\" at file %s, line %d\n>> %s\n", gluErrorString(err), file, line, call);
	
			return !!err;
		}
	
		// return true on error
		bool ILCheck(const char* file, int line, const char* call)
		{
			ILuint err = ilGetError();
			if (err != IL_NO_ERROR)
				gLog.Err("DevIL error: \"%ls\" at file %s, line %d\n>> %s\n", iluErrorString(err), file, line, call);
	
			return !!err;
		}
	}
}
