cmake_minimum_required(VERSION 3.16)

project(Shoal_Fish_Animation LANGUAGES CXX CUDA)
set(CMAKE_CUDA_ARCHITECTURES 86)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

add_executable(Shoal_Fish_Animation
        src/main.cpp
        src/gui/mainWindow.cpp
        src/model/Vector.cpp
        src/model/Vector.h
        src/model/Fish.cpp
        src/model/Fish.h
        src/gui/FishCanvas.cpp
        src/gui/FishCanvas.h
        src/gpu/kernel.cu
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-window sfml-system)