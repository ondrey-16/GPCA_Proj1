//
// Created by ondrey on 31.10.2025.
//

#include <QApplication>
#include <iostream>
#include "gui/mainWindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    int ifGPU;
    if (argc != 2) {
        ifGPU = 1;
    }
    else {
        atoi(argv[1]);
        if (ifGPU != 0 && ifGPU != 1) {
            ifGPU = 1;
        }
    }
    mainWindow w(ifGPU);
    w.show();
    return app.exec();
}
