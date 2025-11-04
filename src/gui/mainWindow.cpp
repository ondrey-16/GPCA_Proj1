//
// Created by ondrey on 31.10.2025.
//

// You may need to build the project (run Qt uic code generator) to get "ui_mainWindow.h" resolved

#include "mainWindow.h"
#include "ui_mainWindow.h"


mainWindow::mainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::mainWindow) {
    ui->setupUi(this);
    this->setWindowTitle("Shoal Fish Animation");
    this->resize(2000, 1200);

    bitmap = new QLabel(this);
    bitmap->setGeometry(0, 0, this->width(), this->height());
    bitmap->setStyleSheet("background-color: blue");
}

void mainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    if (bitmap) {
        bitmap->resize(this->width(), this->height());
    }
}

mainWindow::~mainWindow() {
    delete ui;
    delete bitmap;
}
