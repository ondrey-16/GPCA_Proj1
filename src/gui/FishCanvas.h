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

class FishCanvas : public QWidget {
public:
    FishCanvas(QWidget* parent = nullptr);
    ~FishCanvas();
private:
    void paintEvent(QPaintEvent *) override;
    void onUpdate();
    std::vector<std::vector<std::vector<Fish*>>> fishesOnGrid;
    std::vector<Fish*> shoal;
    float fishesSize = 3.0;
    QTimer* timer;
};


#endif //SHOAL_FISH_ANIMATION_FISHCANVAS_H