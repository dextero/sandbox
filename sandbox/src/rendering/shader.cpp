#include "shader.h"
#include "sharedVertexBuffer.h"
#include "../resources/resourceMgr.h"
#include "../utils/libUtils.h"
#include "../utils/stringUtils.h"
#include "../utils/profiler.h"
#include "../utils/logger.h"

#include <cstring>

namespace sb
{
	std::vector<Shader> Shader::msShaders = std::vector<Shader>();
	std::vector<std::vector<std::pair<uint, std::string> > > Shader::msAttribs;
	Shader::EShader	Shader::msCurrent = Shader::ShaderCount;
	
	GLenum Shader::TranslateShaderType(EShaderType type)
	{
		switch (type)
		{
		case ShaderTypeVertex:		return GL_VERTEX_SHADER;
		case ShaderTypeFragment:	return GL_FRAGMENT_SHADER;
		case ShaderTypeGeometry:	return GL_GEOMETRY_SHADER;
		default:
			assert(!"Invalid shader type!");
			gLog.Err("Invalid shader type: %d. Assuming it's a vertex shader\n");
			return GL_VERTEX_SHADER;
		}
	}

	bool Shader::CheckCompilationStatus(ShaderId shader)
	{
		PROFILE();
	
		GLint retval;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &retval);
		if (retval == GL_FALSE)
		{
			// compilation failed!
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &retval);
	
			gLog.Err("Compilation failed! Log:\n");
			if (retval > 0)
			{
				char* buffer = new char[retval];
	
				glGetShaderInfoLog(shader, retval - 1, &retval, buffer);
				gLog.Info("%s\n", buffer);
				
				delete[] buffer;
			}
	
			return false;
		}
	
		return true;
	}
	
	bool Shader::CheckLinkStatus(ProgramId program)
	{
		PROFILE();
	
		GLint retval;
		glGetProgramiv(program, GL_LINK_STATUS, &retval);
		if (retval == GL_FALSE)
		{
			// link failed!
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &retval);
	
			gLog.Err("Linking failed! Log:\n");
			if (retval > 0)
			{
				char* buffer = new char[retval];
	
				glGetProgramInfoLog(program, retval - 1, &retval, buffer);
				gLog.Info("%s\n", buffer);
				
				delete[] buffer;
			}
	
			return false;
		}
	
		return true;
	}
	
	
	Shader::Shader(): mProgram(0)
	{
		for (int i = 0; i < ShaderTypesCount; ++i)
			mShaders[i] = 0;
	}
	
	Shader::~Shader()
	{
		Free();
	}
	
	bool Shader::LoadShader(EShaderType type, const char* file)
	{
		PROFILE();
	
		std::string path = gResourceMgr.GetShaderPath() + file;
	
	#define _CRT_SECURE_NO_WARNINGS
		FILE* f = fopen(path.c_str(), "r");
	#undef _CRT_SECURE_NO_WARNINGS
	
		if (!f)
			return false;
	
		fseek(f, 0, SEEK_END);
		int filesize = ftell(f);
		fseek(f, 0, SEEK_SET);
	
		char* buffer = new char[filesize];
		memset(buffer, 0, filesize * sizeof(char));
	
		fread(buffer, 1, filesize, f);
		fclose(f);
	
		if (mShaders[type])
			GL_CHECK(glDeleteShader(mShaders[type]));
	
		GL_CHECK(mShaders[type] = glCreateShader(TranslateShaderType(type)));
		GL_CHECK(glShaderSource(mShaders[type], 1, (const GLchar**)&buffer, NULL));
	
		delete[] buffer;
		return true;
	}
	
	bool Shader::Load(const char* vertex, const char* fragment, const char* geometry)
	{
		PROFILE();
	
		gLog.Info("loading shader: %s, %s, %s\n", vertex, fragment, geometry ? geometry : "(no geometry shader)");
	
		bool ret = LoadShader(ShaderTypeVertex, vertex);
		ret &= LoadShader(ShaderTypeFragment, fragment);
		if (geometry) ret &= LoadShader(ShaderTypeGeometry, geometry);
	
		return ret;
	}
	
	bool Shader::CompileAndLink()
	{
		PROFILE();
	
		static const char* shaders[] = { "vertex", "fragment", "geometry" };
		for (int i = ShaderTypeVertex; i != ShaderTypesCount; ++i)
		{
			if (mShaders[i])
			{
				gLog.Info("compiling %s shader...\n", shaders[i]);
				GL_CHECK(glCompileShader(mShaders[i]));
				if (!CheckCompilationStatus(mShaders[i]))
					return false;
			}
		}
	
		if (mProgram)
			GL_CHECK(glDeleteProgram(mProgram));
		mProgram = GL_CHECK(glCreateProgram());

		for (int i = ShaderTypeVertex; i != ShaderTypesCount; ++i)
			if (mShaders[i])
				GL_CHECK(glAttachShader(mProgram, mShaders[i]));
	
		gLog.Info("linking shader program...\n");
		GL_CHECK(glLinkProgram(mProgram));
	
		return CheckLinkStatus(mProgram);
	}
	
	void Shader::Free()
	{
		for (int i = ShaderTypeVertex; i != ShaderTypesCount; ++i)
			if (mShaders[i])
			{
				GL_CHECK(glDeleteShader(mShaders[i]));
				mShaders[i] = 0;
			}

		if (mProgram)
		{
			GL_CHECK(glDeleteProgram(mProgram));
			mProgram = 0;
		}
	}
	
	#define SET_UNIFORM(funccall) \
		PROFILE(); \
		if (!mProgram) return false; \
		GLint loc = glGetUniformLocation(mProgram, name); \
		if (loc == -1) return false; \
		GL_CHECK(funccall); \
		return true
	
	bool Shader::SetUniform(const char* name, const float* value_array, uint elements)
	{
		SET_UNIFORM(glUniform1fv(loc, elements, (const GLfloat*)value_array));
	}
	
	bool Shader::SetUniform(const char* name, const Vec2* value_array, uint elements)
	{
		SET_UNIFORM(glUniform2fv(loc, elements, (const GLfloat*)value_array));
	}
	
	bool Shader::SetUniform(const char* name, const Vec3* value_array, uint elements)
	{
		SET_UNIFORM(glUniform3fv(loc, elements, (const GLfloat*)value_array));
	}
	
	bool Shader::SetUniform(const char* name, const Color* value_array, uint elements)
	{
		SET_UNIFORM(glUniform4fv(loc, elements, (const GLfloat*)value_array));
	}
	
	bool Shader::SetUniform(const char* name, const Mat44* value_array, uint elements)
	{
		SET_UNIFORM(glUniformMatrix4fv(loc, elements, GL_FALSE, (const GLfloat*)value_array));
	}
	
	bool Shader::SetUniform(const char* name, const int* value_array, uint elements)
	{
		SET_UNIFORM(glUniform1iv(loc, elements, (const GLint*)value_array));
	}
	
	void Shader::InitShaders()
	{
		PROFILE();
	
		gLog.Info("initializing shaders...\n");
	
		msShaders.resize(ShaderCount);
		msAttribs.resize(ShaderCount);
	
		msShaders[ShaderTexture].Load("proj_texture.vert", "texture.frag");
		msShaders[ShaderColor].Load("proj_color.vert", "color.frag");
		msShaders[ShaderPointSprite].Load("proj_texture.vert", "color.frag", "point_sprite.geom");
	
		// start from 1, omitting ShaderNone
		for (uint i = 1; i < ShaderCount; ++i)
			msShaders[i].CompileAndLink();
	
		msAttribs[ShaderTexture].push_back(std::make_pair(SharedVertexBuffer::BufferVertex, "a_vertex"));
		msAttribs[ShaderTexture].push_back(std::make_pair(SharedVertexBuffer::BufferColor, "a_color"));
		msAttribs[ShaderTexture].push_back(std::make_pair(SharedVertexBuffer::BufferTexcoord, "a_texcoord"));
	
		msAttribs[ShaderColor].push_back(std::make_pair(SharedVertexBuffer::BufferVertex, "a_vertex"));
		msAttribs[ShaderColor].push_back(std::make_pair(SharedVertexBuffer::BufferColor, "a_color"));

		msAttribs[ShaderPointSprite].push_back(std::make_pair(SharedVertexBuffer::BufferVertex, "a_vertex"));
	}
	
	void Shader::FreeShaders()
	{
		msShaders.clear();
		msAttribs.clear();
	
		gLog.Info("all shaders freed\n");
	}
	
	Shader& Shader::Get(EShader shader)
	{
		PROFILE();
	
		return msShaders[shader];
	}
	
	Shader& Shader::GetCurrent()
	{
		PROFILE();
	
		return msShaders[msCurrent];
	}
	
	void Shader::Use(EShader shader)
	{
		PROFILE();
	
		if (msCurrent < ShaderCount)
			for (std::vector<std::pair<uint, std::string> >::iterator it = msAttribs[msCurrent].begin(); it != msAttribs[msCurrent].end(); ++it)
				GL_CHECK(glDisableVertexAttribArray(it->first));
	
		if (shader != msCurrent)
		{
			if (shader == ShaderNone)
				GL_CHECK(glUseProgram(0));
			else
				GL_CHECK(glUseProgram(msShaders[shader].GetProgram()));
			msCurrent = shader;
		}
	
		if ((uint)msCurrent < msAttribs.size())
			for (std::vector<std::pair<uint, std::string> >::iterator it = msAttribs[msCurrent].begin(); it != msAttribs[msCurrent].end(); ++it)
			{
				GL_CHECK(glEnableVertexAttribArray(it->first));
				GL_CHECK(glBindAttribLocation(msShaders[msCurrent].GetProgram(), it->first, it->second.c_str()));
			}
	}
} // namespace sb
