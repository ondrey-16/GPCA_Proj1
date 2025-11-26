//
// Created by ondrey on 26.11.2025.
//

#ifndef SHOAL_FISH_ANIMATION_FISHCANVASGPU_H
#define SHOAL_FISH_ANIMATION_FISHCANVASGPU_H
#include "FishCanvas.h"
#include "../gpu/kernel.h"
#include <iostream>

class FishCanvasGPU : public FishCanvas {
public:
    FishCanvasGPU(QWidget* parent = nullptr);
    ~FishCanvasGPU();
private:
    void paintEvent(QPaintEvent *) override;
    void onUpdate();
    cudaFishesParams shoalGPU;
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