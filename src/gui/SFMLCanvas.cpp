//
// Created by ondrey on 4.11.2025.
//

#include "SFMLCanvas.h"

SFMLCanvas::SFMLCanvas(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocusPolicy(Qt::StrongFocus);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() { this->update(); });
    timer->start(33);
}

QPaintEngine * SFMLCanvas::paintEngine() const {
    return nullptr;
}

void SFMLCanvas::paintEvent(QPaintEvent *event) {
    onUpdate();
    display();
}

void SFMLCanvas::showEvent(QShowEvent *event) {
    sf::RenderWindow::create(winId());
}
