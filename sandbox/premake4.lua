solution 'sandbox'
    configurations { 'debug', 'release' }

    project 'sandbox'
        kind 'consoleapp'
        language 'c++'
        files { 'src/**.h', '**.cpp', '**.cpp.inl' }
        includedirs { 'src' }
        links { 'GL', 'GLU', 'GLEW', 'IL', 'ILU', 'assimp' }

        configuration 'debug'
            defines { 'DEBUG' }
            flags { 'symbols' }

        configuration 'release'
            defines { 'NDEBUG' }
            flags { 'optimize' }

