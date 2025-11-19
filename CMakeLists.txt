cmake_minimum_required(VERSION 3.27)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(lohoTRON LANGUAGES CXX)

# --- Qt setup ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets)
find_package(OGRE REQUIRED)

# --- Executable target ---
qt_add_executable(lohoTRON
    ${SOURCES}
    ${HEADERS}
)

if (APPLE)
    set(OGRE_DIR "/opt/local/share/ogre/Cmake")
    
    target_link_libraries(lohoTRON PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::OpenGLWidgets
        OgreBites
        OgreRTShaderSystem
        OgreMain
        "-framework OgreOverlay"
    )

    include(FindPkgConfig)
    set(OGRE_PLUGIN_DIR "/usr/local/lib")
    set(OGRE_MEDIA_DIR "/opt/local/share/ogre/Media")
elseif (LINUX)
    set(OGRE_DIR "/usr/local/lib/cmake/OGRE")

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

    find_package(PkgConfig REQUIRED)
    set(OGRE_PLUGIN_DIR "/usr/local/lib/OGRE")
    set(OGRE_MEDIA_DIR "/usr/local/share/OGRE/Media")
endif ()

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
elseif(UNIX AND NOT APPLE)
    # Linux/Ubuntu specific settings
    find_package(X11 REQUIRED)
    target_link_libraries(lohoTRON PRIVATE X11 Xxf86vm)
    
    # Additional libraries that might be needed on Ubuntu
    find_library(GL_LIBRARY GL)
    find_library(GLUT_LIBRARY glut)
    if(GL_LIBRARY)
        target_link_libraries(lohoTRON PRIVATE ${GL_LIBRARY})
    endif()
    if(GLUT_LIBRARY)
        target_link_libraries(lohoTRON PRIVATE ${GLUT_LIBRARY})
    endif()
endif()

# --- Install/Deploy setup ---
include(GNUInstallDirs)

install(TARGETS lohoTRON
    BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMALE_INSTALL_LIBDIR}
)

# --- Qt deployment script ---
qt_generate_deploy_app_script(
    TARGET lohoTRON
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})

# --- Copy OGRE plugins and resources for Ubuntu ---
if(UNIX AND NOT APPLE)
    add_custom_command(TARGET lohoTRON POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/plugins.cfg
        ${CMAKE_CURRENT_BINARY_DIR}/bin/plugins.cfg
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/resources.cfg
        ${CMAKE_CURRENT_BINARY_DIR}/bin/resources.cfg
    )
endif()