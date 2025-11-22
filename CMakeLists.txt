cmake_minimum_required(VERSION 3.27)
project(lohoTRON LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets Gui)

set(OGRE_DIR "/usr/local/lib/cmake/OGRE" CACHE PATH "Path to OGRE cmake files")
find_package(OGRE 14.4 REQUIRED COMPONENTS Bites RTShaderSystem Overlay)

find_package(OpenGL REQUIRED)

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

qt_add_executable(lohoTRON
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(lohoTRON PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui

    OgreMain
    OgreBites
    OgreRTShaderSystem
    OgreOverlay

    OpenGL::GL
)

if(OGRE_INCLUDE_DIRS)
    target_include_directories(lohoTRON PRIVATE ${OGRE_INCLUDE_DIRS})
endif()

if(WIN32)
    set_target_properties(lohoTRON PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    set_target_properties(lohoTRON PROPERTIES MACOSX_BUNDLE TRUE)
elseif(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(lohoTRON PRIVATE X11 Xxf86vm)
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
