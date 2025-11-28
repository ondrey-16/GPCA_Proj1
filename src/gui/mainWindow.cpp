//
// Created by ondrey on 31.10.2025.
//

#include "mainWindow.h"

#include "FishCanvasGPU.h"
#include "FishCanvasCPU.h"
#include "ui_mainWindow.h"

// Tworzy układ okna składający się z sześciu sliderów do manipulowania parametrami zachowań ryb, przycisk do
// zatrzymywania/wznawiania animacji oraz canvas, wewnątrz którego renderowany jest układ ryb i to on wywołuje algorytmy
// do modyfikowania położeń ryb na ekranie. Na podstawie parametru ifGPU wybiera, jaki canvas jest tworzony.
mainWindow::mainWindow(int ifGPU, QWidget *parent) : QMainWindow(parent), ui(new Ui::mainWindow), ifGPU(ifGPU) {
    ui->setupUi(this);
    this->setWindowTitle("Shoal Fish Animation");
    this->resize(2000, 1200);

    centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    layout = new QVBoxLayout(centralWidget);

    controlsPanel = new QWidget(this);
    controlsLayout = new QHBoxLayout(controlsPanel);

    alignRedLabel = new QLabel("Alignment Red:", this);
    alignmentRedSlider = new QSlider(Qt::Horizontal, this->centralWidget);
    alignmentRedSlider->setRange(0, 20);

    separationRedLabel = new QLabel("Separation Red:", this);
    separationRedSlider = new QSlider(Qt::Horizontal, this->centralWidget);
    separationRedSlider->setRange(0, 20);

    cohesionRedLabel = new QLabel("Cohesion Red:", this);
    cohesionRedSlider = new QSlider(Qt::Horizontal, this->centralWidget);
    cohesionRedSlider->setRange(0, 20);

    alignGreenLabel = new QLabel("Alignment Green:", this);
    alignmentGreenSlider = new QSlider(Qt::Horizontal, this->centralWidget);
    alignmentGreenSlider->setRange(0, 20);

    separationGreenLabel = new QLabel("Separation Green:", this);
    separationGreenSlider = new QSlider(Qt::Horizontal, this->centralWidget);
    separationGreenSlider->setRange(0, 20);

    cohesionGreenLabel = new QLabel("Cohesion Green:", this);
    cohesionGreenSlider = new QSlider(Qt::Horizontal, this->centralWidget);
    cohesionGreenSlider->setRange(0, 20);

    controlsLayout->addWidget(alignRedLabel);
    controlsLayout->addWidget(alignmentRedSlider);
    controlsLayout->addWidget(separationRedLabel);
    controlsLayout->addWidget(separationRedSlider);
    controlsLayout->addWidget(cohesionRedLabel);
    controlsLayout->addWidget(cohesionRedSlider);

    controlsLayout->addWidget(alignGreenLabel);
    controlsLayout->addWidget(alignmentGreenSlider);
    controlsLayout->addWidget(separationGreenLabel);
    controlsLayout->addWidget(separationGreenSlider);
    controlsLayout->addWidget(cohesionGreenLabel);
    controlsLayout->addWidget(cohesionGreenSlider);

    animationButton = new QPushButton("Start/Stop Animation", this);
    controlsLayout->addWidget(animationButton);

    layout->addWidget(controlsPanel);
    if (ifGPU) {
        canvas = new FishCanvasGPU(this);
    }
    else {
        canvas = new FishCanvasCPU(this);
    }
    layout->addWidget(canvas, 1);

    connect(alignmentRedSlider, &QSlider::valueChanged,
            canvas, &FishCanvas::setAlignmentWeightRed);
    connect(separationRedSlider, &QSlider::valueChanged,
            canvas, &FishCanvas::setSeparationWeightRed);
    connect(cohesionRedSlider, &QSlider::valueChanged,
            canvas, &FishCanvas::setCohesionWeightRed);
    connect(alignmentGreenSlider, &QSlider::valueChanged,
            canvas, &FishCanvas::setAlignmentWeightGreen);
    connect(separationGreenSlider, &QSlider::valueChanged,
            canvas, &FishCanvas::setSeparationWeightGreen);
    connect(cohesionGreenSlider, &QSlider::valueChanged,
            canvas, &FishCanvas::setCohesionWeightGreen);
    connect(animationButton, &QPushButton::clicked,
            canvas, &FishCanvas::toggleAnimation);

    alignmentRedSlider->setValue(10);
    separationRedSlider->setValue(16);
    cohesionRedSlider->setValue(8);
    alignmentGreenSlider->setValue(10);
    separationGreenSlider->setValue(16);
    cohesionGreenSlider->setValue(8);
}

mainWindow::~mainWindow() {
    delete ui;
}
