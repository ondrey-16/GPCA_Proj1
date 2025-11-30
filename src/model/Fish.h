//
// Created by ondrey on 4.11.2025.
//

#ifndef SHOAL_FISH_ANIMATION_FISH_H
#define SHOAL_FISH_ANIMATION_FISH_H

#include "Vector.h"
#include <vector>
#include <cstdlib>

/*
 * Klasa reprezentująca rybę
 */

class Fish {
public:
    Vector location;
    Vector velocity;
    Vector acceleration;
    static float maxVelocity;
    static float maxForce;
    static float separationWeightRed;
    static float alignmentWeightRed;
    static float cohesionWeightRed;
    static float separationWeightGreen;
    static float alignmentWeightGreen;
    static float cohesionWeightGreen;
    bool isRed;
    Fish() = default;
    Fish(float x, float y, float vx, float vy, bool isRed);
    Vector separate(const std::vector<Fish*>& fishes) const;
    Vector align(const std::vector<Fish*>& fishes) const;
    Vector cohere(const std::vector<Fish*>& fishes) const;
    Vector avoidObstacle(float& cx, float& cy) const;
    bool intersection(float& cx, float& cy, Vector& rayStart, Vector& rayDirection) const;
    void applyForce(const std::vector<Fish*>& fishes, float& cx, float& cy);
    void update(int width, int height);
    void flock(const std::vector<Fish*>& fishes, float& cx, float& cy);
    void borders(int width, int height);
};


#endif //SHOAL_FISH_ANIMATION_FISH_H