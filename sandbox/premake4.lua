solution 'sandbox'
    configurations { 'debug', 'release' }

    project 'sandbox'
        kind 'consoleapp'
        language 'c++'
        files { 'src/**.h', 'src/**.cpp', 'src/**.cpp.inl' }
        includedirs { 'src' }
        targetdir 'bin'
        links { 'GL', 'GLU', 'GLEW', 'IL', 'ILU', 'assimp', 'X11' }
        buildoptions { '-g -Wall -Wextra -std=c++11 -DGLM_FORCE_RADIANS -DGLM_SWIZZLE' }

        configuration 'debug'
            defines { '_DEBUG' }
            flags { 'symbols' }

        configuration 'release'
            defines { 'NDEBUG' }
            flags { 'optimize' }

