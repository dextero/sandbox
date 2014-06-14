solution 'sandbox'
    configurations { 'debug', 'release' }

    project 'sandbox'
        kind 'consoleapp'
        language 'c++'
        files { 'src/**.h', 'src/**.cpp', 'src/**.cpp.inl' }
        includedirs { 'src' }
        targetdir 'bin'
        links { 'GL', 'GLU', 'GLEW', 'IL', 'ILU', 'assimp', 'X11' }
        buildoptions { '-g -Wall -Wextra -Weffc++ -std=c++11 -Wno-pragmas -Wno-unknown-pragmas -Wno-unknown-warning-option -DGLM_FORCE_RADIANS -DGLM_SWIZZLE' }

        configuration 'debug'
            defines { '_DEBUG' }
            flags { 'symbols' }

        configuration 'release'
            defines { 'NDEBUG' }
            flags { 'optimize' }

