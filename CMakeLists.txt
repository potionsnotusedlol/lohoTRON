cmake_minimum_required(VERSION 3.27)
project(lohoTRON LANGUAGES CXX)

# --- Qt setup ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets)

# --- Source files ---
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    SettingsWindow.cpp
    SettingsWindow.h
    GameStartWindow.cpp
    GameStartWindow.h
    ConfigManager.cpp
    ConfigManager.h
    CreatorsWindow.cpp
    CreatorsWindow.h
    resources.qrc
)

# --- Executable target ---
qt_add_executable(lohoTRON
    ${SOURCES}
)

# --- Linking ---
target_link_libraries(lohoTRON PRIVATE Qt6::Core Qt6::Widgets)

# --- Platform-specific tweaks ---
if(WIN32)
    set_target_properties(lohoTRON PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    set_target_properties(lohoTRON PROPERTIES MACOSX_BUNDLE TRUE)
endif()

# --- Install setup ---
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