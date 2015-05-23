cmake_minimum_required(VERSION 2.8.5)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

project(sandbox)

file(GLOB CUSTOM_MODULES "${CMAKE_SOURCE_DIR}/cmake/*")
foreach(MODULE ${CUSTOM_MODULES})
    include(${MODULE})
endforeach()

# setup library directories
if(DEFINED ENV{SDK_DIR})
    file(TO_CMAKE_PATH $ENV{SDK_DIR} SDK_DIR)
    find_library_dir(${SDK_DIR} "assimp")
    find_library_dir(${SDK_DIR} "glew")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DGL_CALL_QUEUE")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -export-dynamic")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# project properties
set(ROOT_DIR ${PROJECT_SOURCE_DIR})
set(LIBRARY_OUTPUT_PATH ${ROOT_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${ROOT_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)

# external libraries
add_external_library(AssImp REQUIRED)
add_external_library(OpenGL REQUIRED)
add_external_library(DevIL REQUIRED)
add_external_library(GLEW REQUIRED)

set(LIBS -ldl ${LIBS} ${IL_LIBRARIES} ${ILU_LIBRARIES} ${ILUT_LIBRARIES})

# project sources
include_directories(${ROOT_DIR}/src)
find_sources(SANDBOX_HEADERS ${ROOT_DIR}/src ".h" "include")
find_sources(SANDBOX_CPP_SOURCES ${ROOT_DIR}/src ".cpp" "src")
set(SANDBOX_SOURCES ${SANDBOX_HEADERS}
                    ${SANDBOX_CPP_SOURCES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11 -Wno-variadic-macros -Wno-pragmas -Wno-unknown-pragmas -DGLM_FORCE_RADIANS -DGLM_SWIZZLE")

add_library(sandbox ${SANDBOX_SOURCES})

# example
find_sources(EXAMPLE_SOURCES ${ROOT_DIR}/example ".cpp" "src")
add_executable(sandbox_example ${EXAMPLE_SOURCES})
target_link_libraries(sandbox_example sandbox ${LIBS})
