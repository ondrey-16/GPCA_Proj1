//
// Created by ondrey on 31.10.2025.
//

#include "mainWindow.h"
#include "ui_mainWindow.h"


mainWindow::mainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::mainWindow) {
    ui->setupUi(this);
    this->setWindowTitle("Shoal Fish Animation");
    this->resize(2000, 1200);

    canvas = new FishCanvas(this);
    canvas->setGeometry(0, 0, this->width(), this->height());
    setCentralWidget(canvas);
}

mainWindow::~mainWindow() {
    delete ui;
    delete canvas;
}
