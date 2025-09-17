cmake_minimum_required(VERSION 3.16)
project(BackMenu VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(third_party/ogre)

set(CEGUI_BUILD_RENDERER_OGREACTIVE ON CACHE BOOL "" FORCE)
set(CEGUI_BUILD_XML_PARSER_TINYXML ON CACHE BOOL "" FORCE)
set(CEGUI_BUILD_STATIC_CONFIGURATION ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/cegui)

add_subdirectory(third_party/ois)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.h)

add_executable(back_menu ${SOURCES} ${HEADERS})

target_include_directories(back_menu PRIVATE
    include
    third_party/ogre/OgreMain/include
    third_party/cegui/cegui/include
    third_party/ois/includes
)

target_link_libraries(back_menu PRIVATE
    OgreMain
    OgreBites
    CEGUIBase
    CEGUIOgreRenderer
    OIS
)
