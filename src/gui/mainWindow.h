//
// Created by ondrey on 31.10.2025.
//

#ifndef SHOAL_FISH_ANIMATION_MAINWINDOW_H
#define SHOAL_FISH_ANIMATION_MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include "FishCanvas.h"
#include <QVBoxLayout>


QT_BEGIN_NAMESPACE

namespace Ui {
    class mainWindow;
}

QT_END_NAMESPACE

// Klasa głównego okna aplikacji
class mainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit mainWindow(int N, int ifCPU, QWidget *parent = nullptr);
    ~mainWindow() override;

private:
    void resizeEvent(QResizeEvent* event) override;
    Ui::mainWindow *ui;
    FishCanvas* canvas;
    QWidget* centralWidget;
    QWidget* controlsPanel;
    QVBoxLayout* layout;
    QHBoxLayout* controlsLayout;
    QLabel* alignRedLabel;
    QLabel* separationRedLabel;
    QLabel* cohesionRedLabel;
    QLabel* alignGreenLabel;
    QLabel* separationGreenLabel;
    QLabel* cohesionGreenLabel;
    QSlider* alignmentRedSlider;
    QSlider* separationRedSlider;
    QSlider* cohesionRedSlider;
    QSlider* alignmentGreenSlider;
    QSlider* separationGreenSlider;
    QSlider* cohesionGreenSlider;
    QPushButton* animationButton;
    int ifGPU;
};


#endif //SHOAL_FISH_ANIMATION_MAINWINDOW_H