cmake_minimum_required(VERSION 3.27)

project(OgreSample LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OGRE, установленный через snap
set(OGRE_DIR "/snap/ogre/current/lib/OGRE/cmake")
find_package(OGRE REQUIRED)

# SDL2, установленный через apt
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Исходники
set(SRCS ./main.cpp)

# Инклуды Ogre (Ogre + OgreBites заголовки)
include_directories("/snap/ogre/current/include/OGRE")

add_executable(${PROJECT_NAME} ${SRCS})

# Линковка OGRE + SDL2
target_link_libraries(${PROJECT_NAME}
    OgreMain
    OgreBites
    OgreRTShaderSystem
    OgreOverlay
    RenderSystem_GL3Plus
    Plugin_ParticleFX
    Codec_STBI
    ${SDL2_LIBRARIES}  # <-- Добавляем SDL2
)

# RPATH, чтобы .so находились без экспорта LD_LIBRARY_PATH
set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "/snap/ogre/current/lib;/snap/ogre/current/lib/OGRE"
    BUILD_WITH_INSTALL_RPATH ON
)

# Конфиги плагинов и ресурсов
set(OGRE_PLUGIN_DIR "/snap/ogre/current/lib/OGRE")
set(OGRE_MEDIA_DIR  "/snap/ogre/current/share/OGRE-14.4/Media")

configure_file(resources.cfg.in resources.cfg @ONLY)
configure_file(plugins.cfg.in   plugins.cfg   @ONLY)
