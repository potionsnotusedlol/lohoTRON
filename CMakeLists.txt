<<<<<<< HEAD
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
=======
cmake_minimum_required(VERSION 3.27)
project(lohoTRON LANGUAGES CXX)

# --- Qt setup ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets)

# --- Source files ---
set(SOURCES
>>>>>>> 4d53333 (rebuild CMakeLists to be cross-platform)
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
    ConfigManager.cpp
    ConfigManager.h
    CreatorsWindow.cpp
    CreatorsWindow.h
<<<<<<< HEAD
    KeyBindingsWindow.cpp
    KeyBindingsWindow.h

)

target_link_libraries(lohoTRON
    PRIVATE
        Qt6::Core
        Qt6::Widgets
=======
    resources.qrc
)

# --- Executable target ---
qt_add_executable(lohoTRON
    ${SOURCES}
>>>>>>> 4d53333 (rebuild CMakeLists to be cross-platform)
)

# --- Linking ---
target_link_libraries(lohoTRON PRIVATE Qt6::Core Qt6::Widgets)

<<<<<<< HEAD
=======
# --- Platform-specific tweaks ---
if(WIN32)
    set_target_properties(lohoTRON PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    set_target_properties(lohoTRON PROPERTIES MACOSX_BUNDLE TRUE)
endif()

# --- Install setup ---
include(GNUInstallDirs)

>>>>>>> 4d53333 (rebuild CMakeLists to be cross-platform)
install(TARGETS lohoTRON
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

<<<<<<< HEAD
if(WIN32)
    qt_generate_deploy_app_script(
        TARGET lohoTRON
        OUTPUT_SCRIPT deploy_script
    )
    install(SCRIPT ${deploy_script})
endif()
=======
# --- Qt deployment script ---
qt_generate_deploy_app_script(
    TARGET lohoTRON
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
>>>>>>> 4d53333 (rebuild CMakeLists to be cross-platform)
