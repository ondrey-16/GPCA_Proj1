//
// Created by ondrey on 4.11.2025.
//

#include "FishCanvas.h"

#include <iostream>

FishCanvas::FishCanvas(QWidget *parent) {
    N = COUNT;
}

FishCanvas::~FishCanvas() {
}

// Funkcje łączące się z zachowaniami sliderów i przycisku do wznawiania/zatrzymywania animacji

void FishCanvas::setAlignmentWeightRed(int value) {
    awR = static_cast<float>(value) / 10.0f;
}

void FishCanvas::setSeparationWeightRed(int value) {
    swR = static_cast<float>(value) / 10.0f;
}

void FishCanvas::setCohesionWeightRed(int value) {
    cwR = static_cast<float>(value) / 10.0f;
}

void FishCanvas::setAlignmentWeightGreen(int value) {
    awG = static_cast<float>(value) / 10.0f;
}

void FishCanvas::setSeparationWeightGreen(int value) {
    swG = static_cast<float>(value) / 10.0f;
}

void FishCanvas::setCohesionWeightGreen(int value) {
    cwG = static_cast<float>(value) / 10.0f;
}

void FishCanvas::toggleAnimation() {
    if (!timer) {
        return;
    }
    if (timer->isActive()) {
        timer->stop();
    }
    else {
        timer->start(33);
    }
}
