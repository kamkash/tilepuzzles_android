cmake_minimum_required(VERSION 3.18.1)
project(tile_puzzles_lib)

set(CMAKE_CXX11_EXTENSION_COMPILE_OPTION "-std=c++17")
set(CMAKE_CXX_STANDARD 17)
option(USE_CLANG "build application with clang" ON) # OFF is the default1
set(CMAKE_VERBOSE_MAKEFILE on)

if (${ANDROID_PLATFORM_LEVEL} LESS 12)
        message(FATAL_ERROR "OpenGL 2 is not supported before API level 11 \
                      (currently using ${ANDROID_PLATFORM_LEVEL}).")
        return()
elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
        add_definitions("-DDYNAMIC_ES3")
        set(GL3STUB_SRC gl3stub.c)
        set(OPENGL_LIB GLESv2)
else ()
        set(OPENGL_LIB GLESv3)
endif (${ANDROID_PLATFORM_LEVEL} LESS 12)

set(SRC
libMain.cpp
AssetUtil.cpp
gl3stub.c
)

include_directories(${CMAKE_SOURCE_DIR}/tilePuzzlesLib/include)

set(LIBS
filament
backend
shaders
vkshaders
ibl
#bluegl
bluevk
filabridge
filaflat
filamat
utils
smol-v
#filamentapp
#filamentapp-resources
#filament-iblprefilter
image
#imageio
camutils
filagui
#imgui
)

#message(FATAL_ERROR "CMAKE SOURCE IS: ${CMAKE_SOURCE_DIR}.")

add_library(tilePuzzlesLib ${SRC} ${GL3STUB_SRC} )
target_compile_options(tilePuzzlesLib PUBLIC -fno-builtin)
target_include_directories(tilePuzzlesLib PUBLIC ${CMAKE_SOURCE_DIR}/tilePuzzlesLib ${CMAKE_SOURCE_DIR}/tilePuzzlesLib/include)

foreach(lib ${LIBS})
        add_library(${lib} STATIC IMPORTED)
        set_target_properties(${lib} PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/tilePuzzlesLib/lib/${ANDROID_ABI}/lib${lib}.a)
        target_link_libraries(tilePuzzlesLib ${lib})
endforeach()

target_link_libraries(tilePuzzlesLib
        ${OPENGL_LIB}
        android
        EGL
        log
        m)


