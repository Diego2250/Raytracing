cmake_minimum_required(VERSION 3.25)
project(Raytracing)

set(CMAKE_CXX_STANDARD 20)
set(SDL2_INCLUDE_DIR C:/Users/DIEAL/OneDrive/Documents/SDL2-2.28.1/include)
set(SDL2_LIB_DIR C:/Users/DIEAL/OneDrive/Documents/SDL2-2.28.1/lib/x64)

include_directories(${SDL2_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})
find_package(SDL2_image CONFIG REQUIRED)

add_executable(Raytracing main.cpp sphere.h sphere.cpp print.h light.h camera.h camera.cpp cube.cpp cube.h skybox.h skybox.cpp)

target_link_libraries(${PROJECT_NAME} SDL2main SDL2 $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)