cmake_minimum_required(VERSION 3.16)

project(Shoal_Fish_Animation LANGUAGES CXX CUDA)
set(CMAKE_CUDA_ARCHITECTURES 86)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets OpenGL)
set(SFML_STATIC_LIBRARIES FALSE)
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
        src/gpu/kernel.h
        src/gui/FishCanvasCPU.cpp
        src/gui/FishCanvasCPU.h
        src/gui/FishCanvasGPU.cpp
        src/gui/FishCanvasGPU.h
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
)

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE ON
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Widgets
        Qt5::OpenGL
        sfml-graphics
        sfml-window
        sfml-system
)