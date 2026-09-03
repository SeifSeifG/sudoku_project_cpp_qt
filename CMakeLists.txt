# CMakeLists.txt for the Qt Widgets Sudoku frontend.
#
# NOTE: I don't have your actual project/root CMakeLists.txt, so the two lines
# marked below (the core library's target name, and its public include
# directory) are placeholders inferred from the #include "core/..." and
# "app/..." paths used throughout the headers you shared. Adjust them to match
# whatever your existing CMakeLists.txt actually calls that library and where
# its headers live.

cmake_minimum_required(VERSION 3.16)
project(sudoku_qt_ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt-specific build machinery.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(sudoku_qt
    main.cpp
    mainwindow.hpp
    mainwindow.cpp
    sudokucell.hpp
    sudokucell.cpp
)

target_link_libraries(sudoku_qt
    PRIVATE
        Qt6::Widgets
        sudoku_core # <-- PLACEHOLDER: replace with your actual core/app library target
)

target_include_directories(sudoku_qt
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include # <-- PLACEHOLDER: wherever core/*.hpp and app/*.hpp actually live
)