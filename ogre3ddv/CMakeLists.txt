cmake_minimum_required(VERSION 3.14)
project(TronBikeDemo14x LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Если OGRE ставили из исходников в /usr/local:
set(OGRE_DIR "/usr/local/lib/cmake/OGRE" CACHE PATH "Path to OGRE cmake files")

find_package(OGRE 14.4 REQUIRED COMPONENTS Bites RTShaderSystem)

add_executable(TronBikeDemo14x main.cpp)
target_link_libraries(TronBikeDemo14x PRIVATE OgreBites OgreRTShaderSystem OgreMain)
# (не обязательно, но можно)
target_include_directories(TronBikeDemo14x PRIVATE ${OGRE_INCLUDE_DIRS})
