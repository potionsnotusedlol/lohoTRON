cmake_minimum_required(VERSION 3.27)
project(lohoTRON LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets)
message("Qt version: ${Qt6_VERSION}")

if(APPLE)
    set(OGRE_DIR "/opt/local/share/ogre/Cmake")
    set(OGRE_PLUGIN_DIR "/usr/local/lib")
    set(OGRE_MEDIA_DIR "/opt/local/share/ogre/Media")
elseif(UNIX)
    set(OGRE_DIR "/usr/local/lib/cmake/OGRE")
    set(OGRE_PLUGIN_DIR "/usr/local/lib/OGRE")
    set(OGRE_MEDIA_DIR "/usr/local/share/OGRE/Media")
    # ensuring OGRE can be found as a snap package 
    find_package(OGRE QUIET)

    if(NOT OGRE_FOUND)
        set(OGRE_DIR "/snap/ogre/current/lib/OGRE/cmake")
        set(OGRE_PLUGIN_DIR "/snap/ogre/current/lib/OGRE")
        set(OGRE_MEDIA_DIR "/snap/ogre/current/share/OGRE-14.4/Media")
        find_package(OGRE REQUIRED)
    endif()
elseif(WIN32)
    set(OGRE_DIR "C:/dev/ogre-contents/build/sdk/CMake")
    set(OGRE_PLUGIN_DIR "C:/dev/ogre-contents/build/sdk/lib")
    set(OGRE_MEDIA_DIR "C:/dev/ogre-contents/build/sdk/Media")
    set(SDL2_DIR "C:/dev/vcpkg/installed/x64-windows/share/sdl2")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/release")
endif()

find_package(OGRE REQUIRED)

# --- Source files ---
set(SOURCES
    ./src/main.cpp
    ./src/mainwindow.cpp
    ./src/MainMenuWidget.cpp
    ./src/MainMenuWidget.ui
    ./src/SettingsWindow.cpp
    ./src/GameStartWindow.cpp
    ./src/QuitGameWindow.cpp
    ./src/CreatorsWindow.cpp
    ./src/GameProcess.cpp
    ./src/KeyCaptureDialog.cpp
    ./src/KeyCaptureProcess.cpp
    ./src/GamePauseWindow.cpp
    resources.qrc
)
set(HEADERS
    ./src/mainwindow.h
    ./src/MainMenuWidget.h
    ./src/SettingsWindow.h
    ./src/GameStartWindow.h
    ./src/QuitGameWindow.h
    ./src/CreatorsWindow.h
    ./src/GameProcess.h
    ./src/KeyCaptureDialog.h
    ./src/KeyCaptureProcess.h
    ./src/GamePauseWindow.h
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
elseif(UNIX OR WIN32)
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
if (WIN32)
    target_link_libraries(lohoTRON PRIVATE
        winmm
        imm32
        version
        opengl32
    )
endif()

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(lohoTRON PRIVATE SDL2::SDL2)
# --- Configuration files for OGRE ---
configure_file(plugins.cfg.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins.cfg @ONLY)
configure_file(resources.cfg.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources.cfg @ONLY)

# --- Platform-specific tweaks ---
if(WIN32)
    set_target_properties(lohoTRON PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    set_target_properties(lohoTRON PROPERTIES MACOSX_BUNDLE TRUE)
endif()

if (UNIX OR APPLE)
    find_package(PkgConfig REQUIRED)
endif()

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(lohoTRON PRIVATE X11 Xxf86vm)
    find_library(GLUT_LIBRARY glut)

    if(GLUT_LIBRARY)
        target_link_libraries(lohoTRON PRIVATE ${GLUT_LIBRARY})
    endif()
endif()

include(GNUInstallDirs)
install(TARGETS lohoTRON
    BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES
    fonts/Bolgarus\ Beta.ttf
    fonts/FREEFATFONT-Regular.otf
    fonts/TraktorMoodFont-Regular.otf
    fonts/Wattauchimma.ttf
    images/bg_menu.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lohoTRON
)
install(DIRECTORY ${OGRE_MEDIA_DIR}/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lohoTRON/Media
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/plugins.cfg
    ${CMAKE_CURRENT_BINARY_DIR}/resources.cfg
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY "${OGRE_PLUGIN_DIR}/"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/OGRE"
    FILES_MATCHING PATTERN "*.so"
)

if(TARGET SDL2::SDL2)
    get_target_property(SDL2_LIB SDL2::SDL2 IMPORTED_LOCATION)

    if(SDL2_LIB)
        install(FILES "${SDL2_LIB}" DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
endif()

include("${Qt6Core_DIR}/Qt6CoreDeploySupport.cmake")
set(DEPLOY_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/deploy_game_script.cmake")
qt_generate_deploy_app_script(
    TARGET lohoTRON
    OUTPUT_SCRIPT DEPLOY_SCRIPT
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${DEPLOY_SCRIPT})