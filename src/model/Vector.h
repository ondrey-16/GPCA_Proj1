//
// Created by ondrey on 4.11.2025.
//

#ifndef SHOAL_FISH_ANIMATION_VECTOR_H
#define SHOAL_FISH_ANIMATION_VECTOR_H

#include <cmath>

class Vector {
public:
    float x;
    float y;
    Vector() = default;
    Vector(float x, float y);
    void set(float x, float y);
    friend Vector operator+(const Vector& v1, const Vector& v2);
    friend Vector operator-(const Vector& v1, const Vector& v2);
    friend Vector operator+(const Vector& v1, const float& s);
    friend Vector operator-(const Vector& v1, const float &s);
    friend Vector operator*(const Vector& v1, const Vector& v2);
    friend Vector operator*(const Vector& v1, const float& s);
    friend Vector operator/(const Vector& v1, const Vector& v2);
    friend Vector operator/(const Vector& v1, const float& s);
    void limit(float max);
    float distance(const Vector& v) const;
    float dotProduct(const Vector& v) const;
    float magnitude() const;
    void setMagnitude(float x);
    float angleBetween(const Vector& v) const;
    Vector copy(const Vector& v);
};


#endif //SHOAL_FISH_ANIMATION_VECTOR_H