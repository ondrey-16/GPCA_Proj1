//
// Created by ondrey on 4.11.2025.
//

#ifndef SHOAL_FISH_ANIMATION_FISH_H
#define SHOAL_FISH_ANIMATION_FISH_H

#include "Vector.h"
#include <vector>
#include <cstdlib>

class Fish {
public:
    Vector location;
    Vector velocity;
    Vector acceleration;
    static float maxVelocity;
    static float maxForce;
    static float separationWeight;
    static float alignmentWeight;
    static float cohesionWeight;
    Fish() = default;
    Fish(float x, float y, float vx, float vy);
    Vector separate(const std::vector<Fish*>& fishes) const;
    Vector align(const std::vector<Fish*>& fishes) const;
    Vector cohere(const std::vector<Fish*>& fishes) const;
    void applyForce(const std::vector<Fish*>& fishes);
    void update();
    void flock(const std::vector<Fish*>& fishes);
    void borders();
};


#endif //SHOAL_FISH_ANIMATION_FISH_H