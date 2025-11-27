//
// Created by ondrey on 26.11.2025.
//

#ifndef SHOAL_FISH_ANIMATION_FISHCANVASGPU_H
#define SHOAL_FISH_ANIMATION_FISHCANVASGPU_H
#include "FishCanvas.h"
#include "../gpu/kernel.h"
#include <iostream>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLContext>
#include <cuda_gl_interop.h>

class FishCanvasGPU : public FishCanvas, protected QOpenGLFunctions_3_3_Core{
    Q_OBJECT
public:
    FishCanvasGPU(QWidget* parent = nullptr);
    ~FishCanvasGPU();
private:
    void initializeGL();
    void paintGL() override;
    void onUpdate();
    cudaArrayPtrs shoalPtrs;
    cudaGraphicsResource_t shoalGPUResource;
    QOpenGLShaderProgram *shaderProgram;
    GLuint vao;
    GLuint vbo;
    int* ifRedOnDevice;
    float* xOnDevice;
    float* yOnDevice;
    float* vxOnDevice;
    float* vyOnDevice;
    float* axOnDevice;
    float* ayOnDevice;
    int* gridIndexesOnDevice;
    int* cellsStartIndexesOnDevice;
    float* nxOnDevice;
    float* nyOnDevice;
    float* nvxOnDevice;
    float* nvyOnDevice;
    int* nifRedOnDevice;
    int* fishesIndexes;
};


#endif //SHOAL_FISH_ANIMATION_FISHCANVASGPU_H