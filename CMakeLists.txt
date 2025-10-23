cmake_minimum_required(VERSION 4.0.2)
project(lohoTRON LANGUAGES CXX)
set(CMAKE_AUTORCC ON)
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

qt_add_executable(lohoTRON
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    SettingsWindow.cpp
    SettingsWindow.h
    GameStartWindow.cpp
    GameStartWindow.h
)

target_link_libraries(lohoTRON
    PRIVATE
        Qt::Core
        Qt::Widgets
)

qt_add_resources(APP_RESOURCES resources.qrc)
target_sources(lohoTRON PRIVATE ${APP_RESOURCES})

include(GNUInstallDirs)

install(TARGETS lohoTRON
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET lohoTRON
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})