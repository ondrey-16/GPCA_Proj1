//
// Created by ondrey on 4.11.2025.
//

#ifndef SHOAL_FISH_ANIMATION_FISHCANVAS_H
#define SHOAL_FISH_ANIMATION_FISHCANVAS_H

#include <cmath>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QTimer>
#include <QOpenGLWidget>
#include <QElapsedTimer>

#define COUNT 10000
/*
 * Klasa, reprezentująca obiekt canvas. Po niej dziedziczą FishCanvasCPU i FishCanvasGPU
 */
class FishCanvas : public QOpenGLWidget {
    Q_OBJECT
public:
    FishCanvas(int N, QWidget* parent = nullptr);
    ~FishCanvas();
    void setAlignmentWeightRed(int value);
    void setSeparationWeightRed(int value);
    void setCohesionWeightRed(int value);
    void setAlignmentWeightGreen(int value);
    void setSeparationWeightGreen(int value);
    void setCohesionWeightGreen(int value);
    void toggleAnimation();
    void onResize(int w, int h);
protected:
    QElapsedTimer frameTimer;
    bool firstFrame = true;
    float fishesSize = 3.0;
    QTimer* timer;
    QPoint* pos;
    int N;
    float awR;
    float swR;
    float cwR;
    float awG;
    float swG;
    float cwG;
    int canvasWidth;
    int canvasHeight;
};


#endif //SHOAL_FISH_ANIMATION_FISHCANVAS_H