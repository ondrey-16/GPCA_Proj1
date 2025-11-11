//
// Created by ondrey on 4.11.2025.
//

#ifndef SHOAL_FISH_ANIMATION_FISHCANVAS_H
#define SHOAL_FISH_ANIMATION_FISHCANVAS_H

#include <vector>

#include "SFMLCanvas.h"
#include "../model/Fish.h"

class FishCanvas : public SFMLCanvas {
public:
    std::vector<Fish*> shoal;
    std::vector<sf::CircleShape> fishesShapes;
    float fishesSize = 3.0;
    FishCanvas(QWidget* parent = nullptr);
    ~FishCanvas();
private:
    void onUpdate() override;
    std::vector<std::vector<std::vector<Fish*>>> fishesOnGrid;
};


#endif //SHOAL_FISH_ANIMATION_FISHCANVAS_H