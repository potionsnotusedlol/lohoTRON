cmake_minimum_required(VERSION 3.16)
project(lohoTRON LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.0 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(lohoTRON
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    SettingsWindow.cpp
    SettingsWindow.h
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
    ConfigManager.cpp
    ConfigManager.h
    CreatorsWindow.cpp
    CreatorsWindow.h
    KeyBindingsWindow.cpp
    KeyBindingsWindow.h

)

target_link_libraries(lohoTRON
    PRIVATE
        Qt6::Core
        Qt6::Widgets
)

qt_add_resources(APP_RESOURCES resources.qrc)
target_sources(lohoTRON PRIVATE ${APP_RESOURCES})

>>>>>>> 4d53333 (rebuild CMakeLists to be cross-platform)
install(TARGETS lohoTRON
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

if(WIN32)
    qt_generate_deploy_app_script(
        TARGET lohoTRON
        OUTPUT_SCRIPT deploy_script
    )
    install(SCRIPT ${deploy_script})
endif()