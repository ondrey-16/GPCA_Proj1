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
    float maxVelocity;
    float maxAcceleration;
    float mass;
    Fish() = default;
    Fish(float x, float y);
    void applyForce(const Vector& force);
    Vector separate(const std::vector<Fish>& fishes) const;
    Vector align(const std::vector<Fish>& fishes) const;
    Vector cohere(const std::vector<Fish>& fishes);
    Vector seek(const Vector& v);
    void swim(const std::vector<Fish>& fishes);
    void update();
    void flock(const std::vector<Fish>& fishes);
    void borders();
    float angle(const Vector& v);
};


#endif //SHOAL_FISH_ANIMATION_FISH_H