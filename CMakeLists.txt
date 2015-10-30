cmake_minimum_required (VERSION 2.8)
project (lluitk)

#
# add local scripts
#
list(APPEND CMAKE_MODULE_PATH
${CMAKE_CURRENT_SOURCE_DIR}/CMake/Packages
${CMAKE_CURRENT_SOURCE_DIR}/submodules/llsg/CMake/Packages
)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(NOT COMPILER_SUPPORTS_CXX11)
       message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

if(APPLE)
   set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++" CACHE STRING "compile flags" FORCE)
   set(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++" CACHE STRING "compile flags" FORCE)
   set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "Deployment target for OSX" FORCE)
endif(APPLE)

if(UNIX)
   set(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "compile flags" FORCE)
endif(UNIX)

#
# opengl
#
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
message("---- opengl ----")
message("OPENGL_INCLUDE_DIR:   ${OPENGL_INCLUDE_DIR}")
message("OPENGL_LIBRARIES:     ${OPENGL_LIBRARIES}")
message("OPENGL_glu_LIBRARY:   ${OPENGL_glu_LIBRARY}")

#
# glew
#
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})
message("---- glew ----")
message("GLEW_INCLUDE_DIR: ${GLEW_INCLUDE_DIR}")
message("GLEW_LIBRARY:     ${GLEW_LIBRARY}")

#
# freetype
#
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})
message("---- freetype ----")
message("FREETYPE_INCLUDE_DIRS:          ${FREETYPE_INCLUDE_DIRS}")
message("FREETYPE_LIBRARIES:             ${FREETYPE_LIBRARIES}")

#
# FreeImage
#
find_package(FreeImage REQUIRED)
include_directories(${FreeImage_INCLUDE_DIRS})
message("---- FreeImage ----")
message("FREEIMAGE_INCLUDE_DIRS:  ${FREEIMAGE_INCLUDE_DIRS}")
message("FREEIMAGE_LIBRARIES:     ${FREEIMAGE_LIBRARIES}")

#
# GLFW
#
find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIR})
message("---- glfw ----")
message("GLFW_INCLUDE_DIR:  ${GLFW_INCLUDE_DIR}")
message("GLFW_LIBRARIES:    ${GLFW_LIBRARIES}")

add_subdirectory (submodules/llsg)
add_subdirectory (src/lluitk)
add_subdirectory (examples)

