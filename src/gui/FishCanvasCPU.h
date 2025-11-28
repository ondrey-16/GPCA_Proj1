//
// Created by ondrey on 26.11.2025.
//

#ifndef SHOAL_FISH_ANIMATION_FISHCANVASCPU_H
#define SHOAL_FISH_ANIMATION_FISHCANVASCPU_H
#include <vector>
#include <iostream>
#include "FishCanvas.h"
#include "../model/Fish.h"

/*
 * Klasa reprezentująca canvas, dla którego wywoływane są algorytmy w wersji CPU
 */
class FishCanvasCPU : public FishCanvas {
public:
    FishCanvasCPU(int N, QWidget* parent = nullptr);
    ~FishCanvasCPU();
private:
    void paintEvent(QPaintEvent *) override;
    void onUpdate();
    std::vector<Fish*> shoal;
    std::vector<std::vector<std::vector<Fish*>>> fishesOnGrid;
};


#endif //SHOAL_FISH_ANIMATION_FISHCANVASCPU_H