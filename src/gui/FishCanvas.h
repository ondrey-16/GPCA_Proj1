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
    FishCanvas(int ifCPU, QWidget* parent = nullptr);
    ~FishCanvas();
    void setAlignmentWeightRed(int value);
    void setSeparationWeightRed(int value);
    void setCohesionWeightRed(int value);
    void setAlignmentWeightGreen(int value);
    void setSeparationWeightGreen(int value);
    void setCohesionWeightGreen(int value);
    void paintCPU(QPainter* painter);
    void paintGPU(QPainter* painter);
    void updateCPU(float cx, float cy);
    void updateGPU(float cx, float cy);
private:
    void paintEvent(QPaintEvent *) override;
    void onUpdate();
    std::vector<std::vector<std::vector<Fish*>>> fishesOnGrid;
    std::vector<Fish*> shoal;
    cudaFishesParams shoalGPU;
    float fishesSize = 3.0;
    QTimer* timer;
    QPoint* pos;
    int N;
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
    float awR;
    float swR;
    float cwR;
    float awG;
    float swG;
    float cwG;
    int ifGPU;
};


#endif //SHOAL_FISH_ANIMATION_FISHCANVAS_H