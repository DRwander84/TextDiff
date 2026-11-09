cmake_minimum_required(VERSION 3.21)

project(TextDiff VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.12 REQUIRED COMPONENTS Widgets Concurrent)

qt_standard_project_setup()

set(TEXTDIFF_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/DiffAlgo.h
    src/DiffEngine.cpp
    src/DiffEngine.h
    src/CodeEditor.cpp
    src/CodeEditor.h
    src/DiffView.cpp
    src/DiffView.h
    src/FilePane.cpp
    src/FilePane.h
    src/GlassPanel.cpp
    src/GlassPanel.h
    src/TextLoader.cpp
    src/TextLoader.h
    src/Theme.h
    src/WindowChrome.cpp
    src/WindowChrome.h
    resources/resources.qrc
)

if(WIN32)
    list(APPEND TEXTDIFF_SOURCES resources/app.rc)
endif()

qt_add_executable(TextDiff WIN32 ${TEXTDIFF_SOURCES})

target_link_libraries(TextDiff PRIVATE Qt6::Widgets Qt6::Concurrent)
