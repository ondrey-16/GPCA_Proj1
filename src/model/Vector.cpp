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

// Modyfikuje wektor, aby nie przekraczał dozwolonej długości max
void Vector::limit(float max) {
    float size = magnitude();
    if (size > max) {
        setMagnitude(max);
    }
}

// Zwraca odległość od drugiego punktu/wektora
float Vector::distance(const Vector &v) const {
    float dx = this->x - v.x;
    float dy = this->y - v.y;
    float dist = std::sqrt(dx*dx + dy*dy);
    return dist;
}

// Iloczyn skalarny dwóch wektorów
float Vector::dotProduct(const Vector &v) const {
    return this->x * v.x + this->y * v.y;
}

// Długość wektora
float Vector::magnitude() const {
    return std::sqrt(this->x * this->x + this->y * this->y);
}

// Skaluje wektor do żądanej długości
void Vector::setMagnitude(float x) {
    if (this->magnitude() > 0) {
        float scale = x / this->magnitude();
        *this = *this * scale;
    }
}

// Zwraca kąt między dwoma wektorami
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

// Kopiowanie wektora
Vector Vector::copy(const Vector &v) {
    return {v.x, v.y};
}

// Dodawanie wektorów
Vector operator+(const Vector &v1, const Vector &v2) {
    return {v1.x + v2.x, v1.y + v2.y};
}

// Odejmowanie wektorów
Vector operator-(const Vector &v1, const Vector &v2) {
    return {v1.x - v2.x, v1.y - v2.y};
}

// Dodanie skalara do obydwu współrzędnych wektora
Vector operator+(const Vector &v1, const float &s) {
    return {v1.x + s, v1.y + s};
}

// Odejmowanie skalara od obydwu współrzędnych wektora
Vector operator-(const Vector &v1, const float &s) {
    return {v1.x - s, v1.y - s};
}

// Mnożenie dwóch wektorów
Vector operator*(const Vector &v1, const Vector &v2) {
    return {v1.x * v2.x, v1.y * v2.y};
}

// Mnożenie wektora przez skalar
Vector operator*(const Vector &v1, const float &s) {
    return {v1.x * s, v1.y * s};
}

// Dzielenie dwóch wektorów
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

// Dzielenie wektora przez skalar
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
