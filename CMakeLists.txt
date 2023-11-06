cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)

project(KDGUI)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
file(GLOB CPP_FILES 
    "src/*.cpp" 

    "inc/*.h"
    "inc/**/*.h"
    "inc/**/**/*.h"
    
    "inc/*.inl"
    "inc/**/*.inl"
    "inc/**/**/*.inl")

include_directories(KDGUI "inc/")
add_executable(KDGUI ${CPP_FILES})

target_precompile_headers(KDGUI PRIVATE inc/precomp.h)
target_link_libraries(KDGUI PRIVATE d3d11 D3DCompiler dxguid)
