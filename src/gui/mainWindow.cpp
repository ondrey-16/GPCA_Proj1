//
// Created by ondrey on 31.10.2025.
//

#include "mainWindow.h"

#include "FishCanvasGPU.h"
#include "FishCanvasCPU.h"
#include "ui_mainWindow.h"


mainWindow::mainWindow(int ifGPU, QWidget *parent) : QMainWindow(parent), ui(new Ui::mainWindow), ifGPU(ifGPU) {
    ui->setupUi(this);
    this->setWindowTitle("Shoal Fish Animation");
    this->resize(2000, 1200);

    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    QWidget* controlsPanel = new QWidget(this);
    QHBoxLayout* controlsLayout = new QHBoxLayout(controlsPanel);

    QLabel* alignRedLabel = new QLabel("Alignment Red:", this);
    alignmentRedSlider = new QSlider(Qt::Horizontal, this->centralWidget());
    alignmentRedSlider->setRange(0, 20);

    QLabel* separationRedLabel = new QLabel("Separation Red:", this);
    separationRedSlider = new QSlider(Qt::Horizontal, this->centralWidget());
    separationRedSlider->setRange(0, 20);

    QLabel* cohesionRedLabel = new QLabel("Cohesion Red:", this);
    cohesionRedSlider = new QSlider(Qt::Horizontal, this->centralWidget());
    cohesionRedSlider->setRange(0, 20);

    QLabel* alignGreenLabel = new QLabel("Alignment Green:", this);
    alignmentGreenSlider = new QSlider(Qt::Horizontal, this->centralWidget());
    alignmentGreenSlider->setRange(0, 20);

    QLabel* separationGreenLabel = new QLabel("Separation Green:", this);
    separationGreenSlider = new QSlider(Qt::Horizontal, this->centralWidget());
    separationGreenSlider->setRange(0, 20);

    QLabel* cohesionGreenLabel = new QLabel("Cohesion Green:", this);
    cohesionGreenSlider = new QSlider(Qt::Horizontal, this->centralWidget());
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
    delete canvas;
}
