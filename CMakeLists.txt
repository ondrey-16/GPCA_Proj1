cmake_minimum_required(VERSION 3.16)

project(Shoal_Fish_Animation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

add_executable(Shoal_Fish_Animation
        src/main.cpp
        src/gui/mainWindow.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)