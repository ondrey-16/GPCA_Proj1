cmake_minimum_required(VERSION 3.16)

project(Shoal_Fish_Animation LANGUAGES CXX CUDA)
enable_language(CUDA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CUDA_ARCHITECTURES native)
set(CMAKE_CUDA_COMPILER nvcc)
find_package(CUDAToolkit REQUIRED)

set(Qt6_DIR "${PROJECT_SOURCE_DIR}/external/Qt/lib/cmake/Qt6")

find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    Gui
    Core
    OpenGL
    OpenGLWidgets
)

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

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/external/Qt/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Gui
    Qt6::Core
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    pthread dl X11
    CUDA::cudart
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN"
)
