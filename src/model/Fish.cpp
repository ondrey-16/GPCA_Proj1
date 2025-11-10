//
// Created by ondrey on 4.11.2025.
//

#include "Fish.h"

Fish::Fish(float x, float y) {
    acceleration = Vector(0, 0);
    velocity = Vector(std::rand() % 3 - 2, std::rand() % 3 - 2);
    location = Vector(x, y);
    maxVelocity = 3.5;
    maxAcceleration = 5;
    mass = 0.1;
}

void Fish::applyForce(const Vector &force) {
    acceleration = acceleration + force / mass;
}

Vector Fish::separate(const std::vector<Fish> &fishes) const {
    float desiredSeparation = 10;
    Vector steer(0, 0);
    int count = 0;

    for (auto fish : fishes) {
        float dist = location.distance(fish.location);
        if ((dist > 0) && (dist < desiredSeparation)) {
            Vector diff = location - fish.location;
            diff.normalize();
            diff = diff / dist;
            steer = steer + diff;
            count++;
        }
    }

    if (count > 0) {
        steer = steer / static_cast<float>(count);
    }
    if (steer.magnitude() > 0) {
        steer.normalize();
        steer = steer * maxVelocity;
        steer = steer - velocity;
        steer.limit(maxAcceleration * mass);
    }
    return steer;
}

Vector Fish::align(const std::vector<Fish> &fishes) const {
    float neighborDist = 50.0;
    Vector sum(0, 0);
    int count = 0;
    for (auto fish : fishes) {
        float dist = location.distance(fish.location);
        if ((dist > 0) && (dist < neighborDist)) {
            sum = sum + fish.velocity;
            count++;
        }
    }

    if (count > 0) {
        sum = sum / static_cast<float>(count);
        sum.normalize();
        sum = sum * maxVelocity;
        Vector steer = sum - velocity;
        steer.limit(maxAcceleration * mass);
        return steer;
    }
    return {0, 0};
}

Vector Fish::cohere(const std::vector<Fish> &fishes) {
    float neighborDist = 50.0;
    Vector sum(0, 0);
    int count = 0;
    for (auto fish : fishes) {
        float dist = location.distance(fish.location);
        if ((dist > 0) && (dist < neighborDist)) {
            sum = sum + fish.location;
            count++;
        }
    }
    if (count > 0) {
        sum = sum / static_cast<float>(count);
        return seek(sum);
    }
    return {0, 0};
}

Vector Fish::seek(const Vector &v) {
    Vector desired = Vector(0, 0) - v;
    desired.normalize();
    desired = desired * maxVelocity;
    acceleration = desired - velocity;
    acceleration.limit(maxAcceleration);
    return acceleration;
}

void Fish::swim(const std::vector<Fish> &fishes) {
    flock(fishes);
    update();
    borders();
}

void Fish::update() {
    acceleration = acceleration * 0.4;
    velocity = velocity + acceleration;
    velocity.limit(maxVelocity);
    location = location + velocity;
    acceleration = acceleration * 0.0;
}

void Fish::flock(const std::vector<Fish> &fishes) {
    Vector sep = separate(fishes);
    Vector ali = align(fishes);
    Vector coh = cohere(fishes);
    sep = sep * 1.5;
    ali = ali * 1.0;
    coh = coh * 1.0;
    applyForce(sep);
    applyForce(ali);
    applyForce(coh);
}

void Fish::borders() {
    if (location.x < 0) {
        location.x = 2000;
        location.y = 1200 - location.y;
    }
    if (location.y < 0) {
        location.y = 1200;
        location.x = 2000 - location.x;
    }
    if (location.x > 2000) {
        location.x = 0;
        location.y = 1200 - location.y;
    }
    if (location.y > 1200) {
        location.y = 0;
        location.x = 2000 - location.x;
    }
}

float Fish::angle(const Vector &v) {
    return std::atan2(v.x, -v.y) * 180 / M_PI;
}
