cmake_minimum_required(VERSION 3.16)
project(Shoal_Fish_Animation LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CUDA_ARCHITECTURES 86)

set(Qt5_DIR "${CMAKE_SOURCE_DIR}/external/Qt/lib/cmake/Qt5")
find_package(Qt5 REQUIRED COMPONENTS Widgets OpenGL Core Gui)

set(SFML_DIR "${CMAKE_SOURCE_DIR}/external/SFML/lib/cmake/SFML")
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME}
        src/main.cpp

        src/gui/mainWindow.cpp
        src/gui/mainWindow.h
        src/gui/mainWindow.ui

        src/model/Vector.cpp
        src/model/Vector.h
        src/model/Fish.cpp
        src/model/Fish.h

        src/gui/FishCanvas.cpp
        src/gui/FishCanvas.h

        src/gpu/kernel.cu
        src/gpu/kernel.h

        src/gui/FishCanvasCPU.cpp
        src/gui/FishCanvasCPU.h
        src/gui/FishCanvasGPU.cpp
        src/gui/FishCanvasGPU.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Widgets
        Qt5::Gui
        Qt5::Core
        Qt5::OpenGL
        sfml-system
        sfml-window
        sfml-graphics
        pthread dl X11
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
)
