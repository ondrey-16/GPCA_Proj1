//
// Created by ondrey on 4.11.2025.
//

#include "Fish.h"

Fish::Fish(float x, float y, float vx, float vy) {
    acceleration = Vector(0, 0);
    location = Vector(x, y);
    maxVelocity = 3.5f;
    velocity = Vector(vx, vy);
    velocity.setMagnitude(maxVelocity);
    maxForce = 0.5f;
}

Vector Fish::separate(const std::vector<Fish*> &fishes) const {
    float neighborDist = 25.0f;
    Vector steer(0, 0);
    float count = 0.0f;

    for (const auto&  fish : fishes) {
        float dist = location.distance(fish->location);
        if (fish != this && dist > 0 && dist < neighborDist) {
            Vector diff = location - fish->location;
            diff = diff / dist;
            steer = steer + diff;
            count++;
        }
    }

    if (count > 0) {
        steer = steer / count;
        steer.setMagnitude(maxVelocity);
        steer = steer - velocity;
        steer.limit(maxForce);
    }
    return steer;
}

Vector Fish::align(const std::vector<Fish*> &fishes) const {
    float neighborDist = 50.0f;
    Vector steer(0, 0);
    float count = 0.0f;
    for (const auto& fish : fishes) {
        float dist = location.distance(fish->location);
        if (fish != this && dist > 0 && dist < neighborDist) {
            steer = steer + fish->velocity;
            count++;
        }
    }
    if (count > 0) {
        steer = steer / count;
        steer.setMagnitude(maxVelocity);
        steer = steer - velocity;
        steer.limit(maxForce);
        return steer;
    }
    return {0, 0};
}

Vector Fish::cohere(const std::vector<Fish*> &fishes) const {
    float neighborDist = 50.0f;
    Vector steer(0, 0);
    float count = 0.0f;
    for (const auto& fish : fishes) {
        float dist = location.distance(fish->location);
        if (fish != this && dist > 0 && dist < neighborDist) {
            steer = steer + fish->location;
            count++;
        }
    }
    if (count > 0) {
        steer = steer / count;
        steer = steer - location;
        steer.setMagnitude(maxVelocity);
        steer = steer - velocity;
        steer.limit(maxForce);
        return steer;
    }
    return {0, 0};
}

void Fish::applyForce(const std::vector<Fish*> &fishes) {
    flock(fishes);
}

void Fish::update() {
    velocity = velocity + acceleration;
    velocity.limit(maxVelocity);
    location = location + velocity;
    acceleration = acceleration * 0.0f;
    borders();
}

void Fish::flock(const std::vector<Fish*> &fishes) {
    Vector sep = separate(fishes);
    Vector ali = align(fishes);
    Vector coh = cohere(fishes);
    acceleration = ali * 1.0f + coh * 0.8f + sep * 1.6f;
}

void Fish::borders() {
    if (location.x > 2000) {
        location.x = 0;
    }
    else if (location.x < 0) {
        location.x = 2000;
    }
    if (location.y > 1200) {
        location.y = 0;
    }
    else if (location.y < 0) {
        location.y = 1200;
    }
}
