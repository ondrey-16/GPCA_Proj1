//
// Created by ondrey on 4.11.2025.
//

#include "Vector.h"

#include <cfloat>

Vector::Vector(float x, float y) {
    this->x = x;
    this->y = y;
}

void Vector::set(float x, float y) {
    this->x = x;
    this->y = y;
}

void Vector::limit(float max) {
    float size = magnitude();
    if (size > max) {
        set(this->x / size, this->y / size);
    }
}

float Vector::distance(const Vector &v) const {
    float dx = this->x - v.x;
    float dy = this->y - v.y;
    float dist = std::sqrt(dx*dx + dy*dy);
    return dist;
}

float Vector::dotProduct(const Vector &v) const {
    return this->x * v.x + this->y * v.y;
}

float Vector::magnitude() const {
    return std::sqrt(this->x * this->x + this->y * this->y);
}

void Vector::setMagnitude(float x) {
    normalize();
    *this = *this * x;
}

float Vector::angleBetween(const Vector &v) const {
    if ((x == 0.0 && y == 0.0) || (v.x == 0.0 && v.y == 0.0)) {
        return 0.0;
    }

    float dot = dotProduct(v);
    float v1Mag = magnitude();
    float v2Mag = v.magnitude();
    float amt = dot / (v1Mag * v2Mag);
    if (amt <= -1) {
        return M_PI;
    }
    if (amt >= 1) {
        return 0.0;
    }
    return (std::acos(amt));
}

void Vector::normalize() {
    float m = magnitude();
    if (m > 0) {
        *this = *this / m;
    }
}

Vector Vector::copy(const Vector &v) {
    return {v.x, v.y};
}

Vector operator+(const Vector &v1, const Vector &v2) {
    return {v1.x + v2.x, v1.y + v2.y};
}

Vector operator-(const Vector &v1, const Vector &v2) {
    return {v1.x - v2.x, v1.y - v2.y};
}

Vector operator+(const Vector &v1, const float &s) {
    return {v1.x + s, v1.y + s};
}

Vector operator-(const Vector &v1, const float &s) {
    return {v1.x - s, v1.y - s};
}

Vector operator*(const Vector &v1, const Vector &v2) {
    return {v1.x * v2.x, v1.y * v2.y};
}

Vector operator*(const Vector &v1, const float &s) {
    return {v1.x * s, v1.y * s};
}

Vector operator/(const Vector &v1, const Vector &v2) {
    Vector v = Vector();
    if (v2.x != 0.0) {
        v.x = v1.x / v2.x;
    }
    else {
        if (v1.x > 0.0) {
            v.x = FLT_MAX;
        }
        else if (v1.x < 0.0) {
            v.x = FLT_MIN;
        }
    }
    if (v2.y != 0.0) {
        v.y = v1.y / v2.y;
    }
    else {
        if (v1.y > 0.0) {
            v.y = FLT_MAX;
        }
        else if (v1.y < 0.0) {
            v.y = FLT_MIN;
        }
    }
    return v;
}

Vector operator/(const Vector &v1, const float &s) {
    if (s != 0.0) {
        return {v1.x / s, v1.y / s};
    }

    Vector v = Vector();
    if (v1.x > 0.0) {
        v.x = FLT_MAX;
    }
    else if (v1.x < 0.0) {
        v.x = FLT_MIN;
    }
    if (v.y > 0.0) {
        v.y = FLT_MAX;
    }
    else if (v.y < 0.0) {
        v.y = FLT_MIN;
    }

    return v;
}
