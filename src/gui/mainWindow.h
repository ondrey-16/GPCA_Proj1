//
// Created by ondrey on 31.10.2025.
//

#ifndef SHOAL_FISH_ANIMATION_MAINWINDOW_H
#define SHOAL_FISH_ANIMATION_MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "FishCanvas.h"


QT_BEGIN_NAMESPACE

namespace Ui {
    class mainWindow;
}

QT_END_NAMESPACE

class mainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit mainWindow(QWidget *parent = nullptr);
    ~mainWindow() override;

private:
    Ui::mainWindow *ui;
    FishCanvas* canvas;
};


#endif //SHOAL_FISH_ANIMATION_MAINWINDOW_H