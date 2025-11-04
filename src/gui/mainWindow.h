//
// Created by ondrey on 31.10.2025.
//

#ifndef SHOAL_FISH_ANIMATION_MAINWINDOW_H
#define SHOAL_FISH_ANIMATION_MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>


QT_BEGIN_NAMESPACE

namespace Ui {
    class mainWindow;
}

QT_END_NAMESPACE

class mainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit mainWindow(QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent *event) override;
    ~mainWindow() override;

private:
    Ui::mainWindow *ui;
    QLabel* bitmap;
};


#endif //SHOAL_FISH_ANIMATION_MAINWINDOW_H