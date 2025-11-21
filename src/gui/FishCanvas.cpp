//
// Created by ondrey on 4.11.2025.
//

#include "FishCanvas.h"

#include <iostream>

FishCanvas::FishCanvas(QWidget *parent) {
    fishesOnGrid.resize(41);
    for (int i = 0; i < 41; i++) {
        fishesOnGrid[i].resize(25);
    }
    N = COUNT;
    for (int i = 0; i < N; i++) {
        int x = rand() % 2000;
        int y = rand() % 1200;
        float vx = rand() % 4000 - 2000;
        float vy = rand() % 2400 - 1200;
        Fish* f = new Fish(x, y, vx, vy);
        shoal.push_back(f);
    }
    initializeFishesParams(N, &shoalGPU);
    initializeMemoryOnGPU(&shoalGPU, &xOnDevice, &yOnDevice, &vxOnDevice, &vyOnDevice, &axOnDevice, &ayOnDevice,
        &nxOnDevice, &nyOnDevice, &nvxOnDevice, &nvyOnDevice, &gridIndexesOnDevice, &cellsStartIndexesOnDevice, &fishesIndexes);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &FishCanvas::onUpdate);
    timer->start(33);
}

FishCanvas::~FishCanvas() {
    for (auto& f : shoal) {
        delete f;
    }
    freeMemoryOnGPU(&xOnDevice, &yOnDevice, &vxOnDevice, &vyOnDevice, &axOnDevice, &ayOnDevice,
        &gridIndexesOnDevice, &cellsStartIndexesOnDevice);
}

void FishCanvas::paintEvent(QPaintEvent *paint_event) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    painter.setBrush(Qt::red);
    painter.setPen(Qt::black);

    // for (auto& fish : shoal) {
    //     painter.drawEllipse(fish->location.x, fish->location.y, 8, 8);
    // }
    for (int i = 0; i < N; i++) {
        painter.drawEllipse(shoalGPU.x[i], shoalGPU.y[i], 8, 8);
    }
}

void FishCanvas::onUpdate() {
    auto start = std::chrono::high_resolution_clock::now();

    // for (int i = 0; i < 41; i++) {
    //     for (int j = 0; j < 25; j++) {
    //         fishesOnGrid[i][j].clear();
    //         fishesOnGrid[i][j].resize(0);
    //     }
    // }
    //
    // for (auto& fish : shoal) {
    //     int fx = std::clamp(static_cast<int>(fish->location.x / 50.f), 0, 40);
    //     int fy = std::clamp(static_cast<int>(fish->location.y / 50.f), 0, 24);
    //     fishesOnGrid[fx][fy].push_back(fish);
    // }
    //
    // for (auto& fish : shoal) {
    //     std::vector<Fish*> shoalPart;
    //     int fx = std::clamp(static_cast<int>(fish->location.x / 50.f), 0, 40);
    //     int fy = std::clamp(static_cast<int>(fish->location.y / 50.f), 0, 24);
    //     for (int i = fx > 0 ? fx - 1 : 0; i < 40 && i <= fx + 1; i++) {
    //         for (int j = fy > 0 ? fy - 1 : 0; j < 24 && j <= fy + 1; j++) {
    //             for (auto& f : fishesOnGrid[i][j]) {
    //                 shoalPart.push_back(f);
    //             }
    //         }
    //     }
    //     fish->applyForce(shoalPart);
    // }
    // for (auto& fish : shoal) {
    //     fish->update();
    // }
    //
    // update();

    updateShoalSceneOnGPU(&shoalGPU, xOnDevice, yOnDevice, vxOnDevice, vyOnDevice, axOnDevice, ayOnDevice,
        nxOnDevice, nyOnDevice, nvxOnDevice, nvyOnDevice, gridIndexesOnDevice, cellsStartIndexesOnDevice, fishesIndexes);
    update();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Update time: " << duration.count() << std::endl;
}
