//
// Created by ondrey on 4.11.2025.
//

#include "Fish.h"

// Parametry wykorzystywane do obliczeń sił działających na ryby
float Fish::maxForce = 0.5f;
float Fish::maxVelocity = 3.5f;
float Fish::separationWeightRed = 1.6f;
float Fish::alignmentWeightRed = 1.0f;
float Fish::cohesionWeightRed = 0.8f;
float Fish::separationWeightGreen = 1.6f;
float Fish::alignmentWeightGreen = 1.0f;
float Fish::cohesionWeightGreen = 0.8f;

Fish::Fish(float x, float y, float vx, float vy, bool isRed) {
    this->acceleration = Vector(0, 0);
    this->location = Vector(x, y);
    this->velocity = Vector(vx, vy);
    this->velocity.setMagnitude(maxVelocity);
    this->isRed = isRed;
}

// Zasada separacji ryb
Vector Fish::separate(const std::vector<Fish*> &fishes) const {
    float neighborDist = 25.0f;
    Vector steer(0, 0);
    float count = 0.0f;

    for (const auto&  fish : fishes) {
        float dist = location.distance(fish->location);
        if (fish != this && isRed == fish->isRed && dist > 0 && dist < neighborDist) {
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
        return steer;
    }
    return {0, 0};
}

// Zasada wyrównywania ryb
Vector Fish::align(const std::vector<Fish*> &fishes) const {
    float neighborDist = 50.0f;
    Vector steer(0, 0);
    float count = 0.0f;

    for (const auto& fish : fishes) {
        float dist = location.distance(fish->location);
        if (fish != this && isRed == fish->isRed && dist > 0 && dist < neighborDist) {
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

// Zasada spajania ryb
Vector Fish::cohere(const std::vector<Fish*> &fishes) const {
    float neighborDist = 50.0f;
    Vector steer(0, 0);
    float count = 0.0f;

    for (const auto& fish : fishes) {
        float dist = location.distance(fish->location);
        if (fish != this && isRed == fish->isRed && dist > 0 && dist < neighborDist) {
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

// Unikanie przeszkody
Vector Fish::avoidObstacle(float& cx, float& cy) const {
    float vMag = velocity.magnitude();
    Vector rayDirection = velocity / vMag;
    Vector rayStart = location;
    if (intersection(cx, cy, rayStart, rayDirection)) {
        Vector fromObstacle(rayStart.x - cx, rayStart.y - cy);
        float fromObstacleMag = fromObstacle.magnitude();
        Vector fromObstacleDirection = fromObstacle / fromObstacleMag;
        Vector avoidForce = fromObstacleDirection * vMag;
        avoidForce.limit(maxForce);
        return avoidForce;
    }
    return {0, 0};
}

// Sprawdzenie, czy obecne położenie i kierunek ruchu ryby przecina się z obszarem wokół przeszkody
bool Fish::intersection(float& cx, float& cy, Vector& rayStart, Vector& rayDirection) const {
    Vector toObstacle(cx - rayStart.x, cy - rayStart.y);
    float proj = rayDirection.x * toObstacle.x + rayDirection.y * toObstacle.y;

    if (proj > 150.f || proj < 0.f) {
        return false;
    }

    Vector distVec = toObstacle - rayDirection * proj;
    if (distVec.magnitude() <= 50.0f) {
        return true;
    }
    return false;
}

// Wywołanie funkcji obliczającej siły
void Fish::applyForce(const std::vector<Fish*> &fishes, float& cx, float& cy) {
    flock(fishes, cx, cy);
}

// Aktualizuje parametry ruchu ryby
void Fish::update(int width, int height) {
    velocity = velocity + acceleration;
    velocity.limit(maxVelocity);
    location = location + velocity;
    acceleration = acceleration * 0.0f;
    borders(width, height);
}

// Na podstawie ryb znajdujących się w bezpośrednim otoczeniu danej ryby obliczane są siły liczone na podstawie
// wszystkich czterech zasad zachowywania się ryb i sumuje je wraz z wagami tych zachowań
void Fish::flock(const std::vector<Fish*> &fishes, float& cx, float& cy) {
    Vector sep = separate(fishes);
    Vector ali = align(fishes);
    Vector coh = cohere(fishes);
    Vector avo = avoidObstacle(cx, cy);
    float aw = (isRed) ? alignmentWeightRed : alignmentWeightGreen;
    float cw = (isRed) ? cohesionWeightRed : cohesionWeightGreen;
    float sw = (isRed) ? separationWeightRed : separationWeightGreen;
    acceleration = ali * aw + coh * cw + sep * sw + avo * 2.0f;
}

// Modyfikuje położenie ryby, aby mieściła się w zakresie okna
void Fish::borders(int width, int height) {
    if (location.x > width) {
        location.x = 0;
    }
    else if (location.x < 0) {
        location.x = width;
    }
    if (location.y > height) {
        location.y = 0;
    }
    else if (location.y < 0) {
        location.y = height;
    }
}
