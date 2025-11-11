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
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(lohoTRON LANGUAGES CXX)

# --- Qt setup ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets)
find_package(OGRE REQUIRED)

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
    QuitGameWindow.cpp
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
>>>>>>> 4d53333 (rebuild CMakeLists to be cross-platform)
    ${HEADERS}
)

# --- Linking ---
set(OGRE_INCLUDE_DIRS "/opt/local/include/OGRE")
set(OGRE_LIBRARIES
    OgreMain
    OgreBites
    OgreRTShaderSystem
    "-framework RenderSystem_GL3Plus"
)
target_link_libraries(lohoTRON PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Qt6::OpenGLWidgets ${OGRE_LIBRARIES})
set(OGRE_PLUGIN_DIR "/opt/local/lib")
set(OGRE_MEDIA_DIR "/opt/local/share/ogre/Media")
configure_file(plugins.cfg.in plugins.cfg @ONLY)
configure_file(resources.cfg.in resources.cfg @ONLY)

<<<<<<< HEAD
=======
# --- Platform-specific tweaks ---
if(WIN32)
    set_target_properties(lohoTRON PROPERTIES WIN32_EXECUTABLE TRUE)
    # include(Windeployqt)
    # windeployqt(some args ion know bru)
elseif(APPLE)
    set_target_properties(lohoTRON PROPERTIES MACOSX_BUNDLE TRUE)
endif()

# --- Install/Deploy setup ---
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
