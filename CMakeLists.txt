cmake_minimum_required(VERSION 3.27)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(lohoTRON LANGUAGES CXX)

# --- Qt setup ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets)

# --- OGRE setup for Ubuntu ---
set(OGRE_DIR "/usr/local/lib/cmake/OGRE" CACHE PATH "Path to OGRE cmake files")
find_package(OGRE 14.4 REQUIRED COMPONENTS Bites RTShaderSystem Overlay)

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

# --- Linking ---
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

target_include_directories(lohoTRON PRIVATE ${OGRE_INCLUDE_DIRS})

# --- SDL2 for Ubuntu ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
if(SDL2_FOUND)
    target_include_directories(lohoTRON PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(lohoTRON PRIVATE ${SDL2_LIBRARIES})
endif()

# --- Configuration files for OGRE ---
set(OGRE_PLUGIN_DIR "/usr/local/lib/OGRE")
set(OGRE_MEDIA_DIR "/usr/local/share/OGRE/Media")
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
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
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