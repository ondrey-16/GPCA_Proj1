//
// Created by ondrey on 4.11.2025.
//

#include "FishCanvas.h"

FishCanvas::FishCanvas(QWidget *parent) : SFMLCanvas(parent) {
    for (int i = 0; i < 500; i++) {
        int x = rand() % 2000;
        int y = rand() % 1200;
        float vx = rand() % 4000 - 2000;
        float vy = rand() % 2400 - 1200;
        Fish f(x, y, vx, vy);
        sf::CircleShape shape(10, 50);
        shape.setFillColor(sf::Color::Red);
        shape.setOutlineColor(sf::Color::Black);
        shape.setRadius(fishesSize);
        shoal.push_back(f);
        fishesShapes.push_back(shape);
    }
}

void FishCanvas::onUpdate() {
    clear();
    for (auto& fish : shoal) {
        fish.applyForce(shoal);
    }
    for (auto& fish : shoal) {
        fish.update();
    }

    for (int i = 0; i < fishesShapes.size(); i++) {
        Fish& f = shoal[i];
        fishesShapes[i].setPosition(f.location.x, f.location.y);
        draw(fishesShapes[i]);
    }
}
