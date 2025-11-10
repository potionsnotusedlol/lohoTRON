cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Имя проекта / exe
set(OGRE_PROJECT_NAME "OgreSample")
project(${OGRE_PROJECT_NAME} LANGUAGES CXX)

# Где лежит Ogre, установленный через snap
set(OGRE_DIR "/snap/ogre/current/lib/OGRE/cmake")
find_package(OGRE REQUIRED)

# --------- исходники ---------
set(SRCS
    ./main.cpp
)

# --------- инклуды Ogre ---------
# (тебе нужны заголовки Ogre и OgreBites)
set(OGRE_INCLUDE_DIRS "/snap/ogre/current/include/OGRE")
include_directories(${OGRE_INCLUDE_DIRS})

# --------- библиотеки Ogre ---------
# ВАЖНО: сюда добавил OgreOverlay, он нужен для TrayManager/HUD
set(OGRE_LIBRARIES
    OgreMain
    OgreBites
    OgreRTShaderSystem
    OgreOverlay
    RenderSystem_GL3Plus
    Plugin_ParticleFX
    Codec_STBI
)

# --------- исполняемый файл ---------
add_executable(${OGRE_PROJECT_NAME} ${SRCS})

target_link_libraries(${OGRE_PROJECT_NAME}
    ${OGRE_LIBRARIES}
)

# RPATH, чтобы .so находились без танцев с LD_LIBRARY_PATH
set_target_properties(${OGRE_PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "/snap/ogre/current/lib;/snap/ogre/current/lib/OGRE"
    BUILD_WITH_INSTALL_RPATH ON
)

# Конфиги плагинов и ресурсов (как у тебя было)
set(OGRE_PLUGIN_DIR "/snap/ogre/current/lib/OGRE")
set(OGRE_MEDIA_DIR "/snap/ogre/current/share/OGRE-14.4/Media")
configure_file(plugins.cfg.in plugins.cfg @ONLY)
configure_file(resources.cfg.in resources.cfg @ONLY)
