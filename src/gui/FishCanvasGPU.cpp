//
// Created by ondrey on 26.11.2025.
//

#include "FishCanvasGPU.h"

FishCanvasGPU::FishCanvasGPU(QWidget *parent) : FishCanvas(parent) {
    initializeFishesParams(N, &shoalGPU);
    initializeMemoryOnGPU(&shoalGPU, &ifRedOnDevice, &xOnDevice, &yOnDevice, &vxOnDevice, &vyOnDevice, &axOnDevice, &ayOnDevice,
        &nxOnDevice, &nyOnDevice, &nvxOnDevice, &nvyOnDevice, &nifRedOnDevice, &gridIndexesOnDevice, &cellsStartIndexesOnDevice, &fishesIndexes);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &FishCanvasGPU::onUpdate);
    timer->start(33);
}

FishCanvasGPU::~FishCanvasGPU() {
    freeMemoryOnGPU(&ifRedOnDevice, &xOnDevice, &yOnDevice, &vxOnDevice, &vyOnDevice, &axOnDevice, &ayOnDevice,
        &nxOnDevice, &nyOnDevice, &nvxOnDevice, &nvyOnDevice, &nifRedOnDevice, &gridIndexesOnDevice, &cellsStartIndexesOnDevice, &fishesIndexes);
}

void FishCanvasGPU::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    painter.setPen(Qt::black);

    for (int i = 0; i < N; i++) {
        if (shoalGPU.ifRed[i] == 1) {
            painter.setBrush(Qt::red);
        }
        else {
            painter.setBrush(Qt::green);
        }
        float angle = std::atan2(shoalGPU.vy[i], shoalGPU.vx[i]) * 180.f / M_PI;
        painter.save();
        painter.translate(shoalGPU.x[i], shoalGPU.y[i]);
        painter.rotate(angle);
        QPolygonF arrow;
        arrow << QPoint(10, 0) << QPointF(-6, -4)
            << QPoint(-3, 0) << QPoint(-6, 4);
        painter.drawPolygon(arrow);
        painter.restore();
    }
}

void FishCanvasGPU::onUpdate() {
    auto start = std::chrono::high_resolution_clock::now();
    QPoint globalPos = QCursor::pos();
    QPoint localPos = this->mapFromGlobal(globalPos);

    float cx = localPos.x();
    float cy = localPos.y();

    updateShoalSceneOnGPU(&shoalGPU, ifRedOnDevice, xOnDevice, yOnDevice, vxOnDevice, vyOnDevice, axOnDevice, ayOnDevice,
        nxOnDevice, nyOnDevice, nvxOnDevice, nvyOnDevice, nifRedOnDevice, gridIndexesOnDevice, cellsStartIndexesOnDevice, fishesIndexes,
        cx, cy, swR, awR, cwR, swG, awG, cwG);

    update();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Update speed: " << 1e9 / duration.count() << " FPS" << std::endl;
}
