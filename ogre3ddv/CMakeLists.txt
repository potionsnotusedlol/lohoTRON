cmake_minimum_required(VERSION 3.14)
project(TronBike_Cubes_Collision LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Укажи OGRE_DIR, если ставил из исходников иначе
set(OGRE_DIR "/usr/local/lib/cmake/OGRE" CACHE PATH "Path to OGRE cmake files")

find_package(OGRE 14.4 REQUIRED COMPONENTS Bites RTShaderSystem)

add_executable(TronBike_Cubes_Collision main.cpp)
target_link_libraries(TronBike_Cubes_Collision PRIVATE OgreBites OgreRTShaderSystem OgreMain)
target_include_directories(TronBike_Cubes_Collision PRIVATE ${OGRE_INCLUDE_DIRS})

# SDL2 обычно не нужен отдельно, OgreBites подтянет
find_package(SDL2 QUIET)
if (SDL2_FOUND)
    target_include_directories(TronBike_Cubes_Collision PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(TronBike_Cubes_Collision PRIVATE ${SDL2_LIBRARIES})
endif()
