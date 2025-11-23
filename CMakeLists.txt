cmake_minimum_required(VERSION 3.27)
project(lohoTRON LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets)
find_package(OGRE REQUIRED)

if(APPLE)
    set(OGRE_DIR "/opt/local/share/ogre/Cmake")
    set(OGRE_PLUGIN_DIR "/usr/local/lib")
    set(OGRE_MEDIA_DIR "/opt/local/share/ogre/Media")
elseif(UNIX)
    set(OGRE_DIR "/usr/local/lib/cmake/OGRE")
    set(OGRE_PLUGIN_DIR "/usr/local/lib/OGRE")
    set(OGRE_MEDIA_DIR "/usr/local/share/OGRE/Media")
endif()

# --- Source files ---
set(SOURCES
    main.cpp
    mainwindow.cpp
    MainMenuWidget.cpp
    MainMenuWidget.ui
    SettingsWindow.cpp
    GameStartWindow.cpp
    QuitGameWindow.cpp
    CreatorsWindow.cpp
    GameProcess.cpp
    resources.qrc
)

set(HEADERS
    mainwindow.h
    MainMenuWidget.h
    SettingsWindow.h
    GameStartWindow.h
    QuitGameWindow.h
    CreatorsWindow.h
    GameProcess.h
)

# --- Executable target ---
qt_add_executable(lohoTRON
    ${SOURCES}
    ${HEADERS}
)

if(APPLE)
    target_link_libraries(lohoTRON PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::OpenGLWidgets
        OgreBites
        OgreRTShaderSystem
        OgreMain
        "-framework OgreOverlay"
        "-framework OpenGL"
    )
elseif(UNIX)
    target_link_libraries(lohoTRON PRIVATE 
        Qt6::Core 
        Qt6::Widgets 
        Qt6::Gui 
        Qt6::OpenGLWidgets 
        OgreBites 
        OgreRTShaderSystem 
        OgreOverlay 
        OgreMain
    )

    find_library(GL_LIBRARY GL)

    if(GL_LIBRARY)
        target_link_libraries(lohoTRON PRIVATE ${GL_LIBRARY})
    endif()
endif()


target_include_directories(lohoTRON PRIVATE ${OGRE_INCLUDE_DIRS})
# --- SDL2 ---
find_package(SDL2 REQUIRED)
target_include_directories(lohoTRON PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(lohoTRON PRIVATE ${SDL2_LIBRARIES})
# --- Configuration files for OGRE ---
configure_file(plugins.cfg.in plugins.cfg @ONLY)
configure_file(resources.cfg.in resources.cfg @ONLY)

# --- Platform-specific tweaks ---
if(WIN32)
    set_target_properties(lohoTRON PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    set_target_properties(lohoTRON PROPERTIES MACOSX_BUNDLE TRUE)
endif()

find_package(PkgConfig REQUIRED)

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(lohoTRON PRIVATE X11 Xxf86vm)
    find_library(GLUT_LIBRARY glut)

    if(GLUT_LIBRARY)
        target_link_libraries(lohoTRON PRIVATE ${GLUT_LIBRARY})
    endif()
endif()

set(OGRE_PLUGIN_DIR "/usr/local/lib/OGRE")
set(OGRE_MEDIA_DIR "/usr/local/share/OGRE/Media")
configure_file(plugins.cfg.in plugins.cfg @ONLY)
configure_file(resources.cfg.in resources.cfg @ONLY)

include(GNUInstallDirs)

install(TARGETS lohoTRON
    BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMALE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET lohoTRON
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})