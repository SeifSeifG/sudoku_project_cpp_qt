cmake_minimum_required(VERSION 3.16)

if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER "/usr/bin/g++-15" CACHE FILEPATH "Path to C++ compiler")
endif()

set(UI_MODE "qt" CACHE STRING "Default UI mode")

project(SudokuProject CXX)

# Automatically handle Qt MOC files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# allow the configure step (and sudoku-term) to succeed
# even when Qt6 isn't installed on this machine.
find_package(Qt6 QUIET COMPONENTS Widgets)

file(GLOB CORE_SOURCES "core/*.cpp")
file(GLOB APP_SOURCES "app/*.cpp")

set(SHARED_SOURCES
    ${CORE_SOURCES}
    ${APP_SOURCES}
    main.cpp
)

# 1. Terminal Target (sudoku-term)
add_executable(sudoku-term
    ${SHARED_SOURCES}
    ui/Terminal/TerminalUI.cpp
)

target_include_directories(sudoku-term PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/Terminal
)

target_compile_options(sudoku-term PRIVATE 
    -std=c++26
    -Wall 
    -Wextra 
    -g 
    -fdiagnostics-color=always
)

target_link_options(sudoku-term PRIVATE -static)


# 2. Qt6 Target (sudoku-qt) — only built if Qt6 was actually found
if(Qt6_FOUND)
    add_executable(sudoku-qt
        ${SHARED_SOURCES}
        ui/QT/MainWindow.cpp
        ui/QT/MainWindow.hpp
        ui/QT/QCell.cpp
        ui/QT/QCell.hpp
    )

    target_compile_definitions(sudoku-qt PRIVATE QT_UI)

    target_include_directories(sudoku-qt PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/QT
    )

    target_compile_options(sudoku-qt PRIVATE 
        -std=c++20
        -Wall 
        -Wextra 
        -g 
        -fPIC 
        -fdiagnostics-color=always
    )

    target_link_libraries(sudoku-qt PRIVATE Qt6::Widgets)
else()
    message(STATUS "Qt6 not found — skipping sudoku-qt target (sudoku-term will still build).")
endif()