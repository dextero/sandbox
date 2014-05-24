solution 'sandbox'
    configurations { 'debug', 'release' }

    project 'sandbox'
        kind 'consoleapp'
        language 'c++'
        files { 'src/**.h', '**.cpp', '**.cpp.inl' }
        includedirs { 'src' }
        targetdir 'bin'
        links { 'GL', 'GLU', 'GLEW', 'IL', 'ILU', 'assimp', 'X11' }
        buildoptions { '-Wall -Wextra -std=c++11 -DGLM_FORCE_RADIANS' }

        configuration 'debug'
            defines { '_DEBUG' }
            flags { 'symbols' }

        configuration 'release'
            defines { 'NDEBUG' }
            flags { 'optimize' }

