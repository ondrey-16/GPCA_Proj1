//
// Created by ondrey on 4.11.2025.
//

#include "FishCanvas.h"

#include <iostream>

FishCanvas::FishCanvas(QWidget *parent) : SFMLCanvas(parent) {
    fishesOnGrid.resize(21);
    for (int i = 0; i < 21; i++) {
        fishesOnGrid[i].resize(13);
    }
    for (int i = 0; i < 500; i++) {
        int x = rand() % 2000;
        int y = rand() % 1200;
        float vx = rand() % 4000 - 2000;
        float vy = rand() % 2400 - 1200;
        Fish* f = new Fish(x, y, vx, vy);
        sf::CircleShape shape(10, 50);
        shape.setFillColor(sf::Color::Red);
        shape.setOutlineColor(sf::Color::Black);
        shape.setRadius(fishesSize);
        shoal.push_back(f);
        fishesShapes.push_back(shape);
    }
}

FishCanvas::~FishCanvas() {
    for (auto& f : shoal) {
        delete f;
    }
}

void FishCanvas::onUpdate() {
    auto start = std::chrono::high_resolution_clock::now();

    clear();

    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 13; j++) {
            fishesOnGrid[i][j].clear();
            fishesOnGrid[i][j].resize(0);
        }
    }

    for (auto& fish : shoal) {
        int fx = std::clamp(static_cast<int>(fish->location.x / 100.f), 0, 20);
        int fy = std::clamp(static_cast<int>(fish->location.y / 100.f), 0, 12);
        fishesOnGrid[fx][fy].push_back(fish);
    }

    for (auto& fish : shoal) {
        std::vector<Fish*> shoalPart;
        int fx = std::clamp(static_cast<int>(fish->location.x / 100.f), 0, 20);
        int fy = std::clamp(static_cast<int>(fish->location.y / 100.f), 0, 12);
        for (int i = fx > 0 ? fx - 1 : 0; i < 21 && i <= fx + 1; i++) {
            for (int j = fy > 0 ? fy - 1 : 0; j < 13 && j <= fy + 1; j++) {
                for (auto& f : fishesOnGrid[i][j]) {
                    shoalPart.push_back(f);
                }
            }
        }
        fish->applyForce(shoalPart);
    }
    for (auto& fish : shoal) {
        fish->update();
    }

    for (int i = 0; i < fishesShapes.size(); i++) {
        Fish* f = shoal[i];
        fishesShapes[i].setPosition(f->location.x, f->location.y);
        draw(fishesShapes[i]);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Update time: " << duration.count() << std::endl;
}
