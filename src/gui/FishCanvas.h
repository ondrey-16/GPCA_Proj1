//
// Created by ondrey on 4.11.2025.
//

#ifndef SHOAL_FISH_ANIMATION_FISHCANVAS_H
#define SHOAL_FISH_ANIMATION_FISHCANVAS_H

#include <vector>
#include <QPainter>
#include <QWidget>
#include <QTimer>

#include "../model/Fish.h"
#include "../gpu/kernel.h"

class FishCanvas : public QWidget {
public:
    FishCanvas(QWidget* parent = nullptr);
    ~FishCanvas();
private:
    void paintEvent(QPaintEvent *) override;
    void onUpdate();
    std::vector<std::vector<std::vector<Fish*>>> fishesOnGrid;
    std::vector<Fish*> shoal;
    cudaFishesParams shoalGPU;
    float fishesSize = 3.0;
    QTimer* timer;
    int N;
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
    int* fishesIndexes;
};


#endif //SHOAL_FISH_ANIMATION_FISHCANVAS_H