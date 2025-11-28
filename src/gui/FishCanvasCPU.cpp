//
// Created by ondrey on 26.11.2025.
//

#include "FishCanvasCPU.h"

// Inicjalizuje dane początkowe obrazujące ruch ryb. Mierzy czas działania inicjalizacji
FishCanvasCPU::FishCanvasCPU(QWidget *parent) : FishCanvas(parent) {
    auto start = std::chrono::high_resolution_clock::now();
    fishesOnGrid.resize(41);
    for (int i = 0; i < 41; i++) {
        fishesOnGrid[i].resize(25);
    }
    for (int i = 0; i < N; i++) {
        float vx = (rand() % 70 - 35) / 10.0f;
        float vy = (rand() % 70 - 35) / 10.0f;
        float x = rand() % 2000;
        float y = rand() % 1200;
        int isRed = i % 2 == 0;
        Fish* f = new Fish(x, y, vx, vy, isRed);
        shoal.push_back(f);
    }
    auto end = std::chrono::high_resolution_clock::now();
    int initDataDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Initializing data on CPU time: " << initDataDuration / 1e9 << "s" << std::endl;
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &FishCanvasCPU::onUpdate);
    timer->start(33);
}

// Zwalnia zaalokowaną pamięć. Mierzy czas zwalniania pamięci
FishCanvasCPU::~FishCanvasCPU() {
    auto start = std::chrono::high_resolution_clock::now();
    for (auto& f : shoal) {
        delete f;
    }
    delete timer;
    auto end = std::chrono::high_resolution_clock::now();
    int freeDataDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Memory release time: " << freeDataDuration / 1e9 << "s" << std::endl;
}

// Mierzy czas trwania renderowania się klatki. Na podstawie parametrów ryb wizualizuje ich położenie.
void FishCanvasCPU::paintEvent(QPaintEvent *paint_event) {
    if (!firstFrame) {
        int duration = frameTimer.nsecsElapsed();
        std::cout << "Frame rendering speed: " << 1e9 / duration << " FPS" << std::endl;
    } else {
        firstFrame = false;
    }

    frameTimer.restart();

    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    painter.setPen(Qt::black);
    for (auto& fish : shoal) {
        if (fish->isRed) {
            painter.setBrush(Qt::red);
        }
        else {
            painter.setBrush(Qt::green);
        }
        float angle = std::atan2(fish->velocity.y, fish->velocity.x) * 180.f / M_PI;
        painter.save();
        painter.translate(fish->location.x, fish->location.y);
        painter.rotate(angle);
        QPolygonF arrow;
        arrow << QPoint(10, 0) << QPointF(-6, -4)
            << QPoint(-3, 0) << QPoint(-6, 4);
        painter.drawPolygon(arrow);
        painter.restore();
    }
}

// Dokonuje modyfikacji położeń ryb
void FishCanvasCPU::onUpdate() {
    Fish::alignmentWeightRed = awR;
    Fish::alignmentWeightGreen = awG;
    Fish::cohesionWeightRed = cwR;
    Fish::cohesionWeightGreen = cwG;
    Fish::separationWeightRed = swR;
    Fish::separationWeightGreen = swG;

    QPoint globalPos = QCursor::pos();
    QPoint localPos = this->mapFromGlobal(globalPos);

    float cx = localPos.x();
    float cy = localPos.y();

    for (int i = 0; i < 41; i++) {
        for (int j = 0; j < 25; j++) {
            fishesOnGrid[i][j].clear();
            fishesOnGrid[i][j].resize(0);
        }
    }

    for (auto& fish : shoal) {
        int fx = std::clamp(static_cast<int>(fish->location.x / 50.f), 0, 40);
        int fy = std::clamp(static_cast<int>(fish->location.y / 50.f), 0, 24);
        fishesOnGrid[fx][fy].push_back(fish);
    }

    for (auto& fish : shoal) {
        std::vector<Fish*> shoalPart;
        int fx = std::clamp(static_cast<int>(fish->location.x / 50.f), 0, 40);
        int fy = std::clamp(static_cast<int>(fish->location.y / 50.f), 0, 24);
        for (int i = fx > 0 ? fx - 1 : 0; i < 40 && i <= fx + 1; i++) {
            for (int j = fy > 0 ? fy - 1 : 0; j < 24 && j <= fy + 1; j++) {
                for (auto& f : fishesOnGrid[i][j]) {
                    shoalPart.push_back(f);
                }
            }
        }
        fish->applyForce(shoalPart, cx, cy);
    }
    for (auto& fish : shoal) {
        fish->update();
    }

    update();
}
