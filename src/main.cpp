//
// Created by ondrey on 31.10.2025.
//

#include <QApplication>
#include <iostream>
#include "gui/mainWindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    if (argc != 2) {
        std::cerr << "Invalid args count";
        exit(1);
    }
    int ifCPU = atoi(argv[1]);
    if (ifCPU != 0 && ifCPU != 1) {
        std::cerr << "Invalid args";
        exit(1);
    }
    mainWindow w(ifCPU);
    w.show();
    return app.exec();
}
