//
// Created by ondrey on 4.11.2025.
//

#include "FishCanvas.h"

FishCanvas::FishCanvas(QWidget *parent) : SFMLCanvas(parent) {
    for (int i = 0; i < 250; i++) {
        Fish f(400, 300);
        sf::CircleShape shape(8, 3);
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
        fish.swim(shoal);
    }

    for (int i = 0; i < fishesShapes.size(); i++) {
        Fish& f = shoal[i];
        fishesShapes[i].setPosition(f.location.x, f.location.y);
        fishesShapes[i].setRotation(f.angle(f.velocity));
        draw(fishesShapes[i]);
    }
}
