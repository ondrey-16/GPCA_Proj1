//
// Created by ondrey on 31.10.2025.
//

#include <QApplication>
#include <iostream>
#include "gui/mainWindow.h"
#include "gpu/kernel.h"


int main(int argc, char** argv) {
    QApplication app(argc, argv);
    if (argc != 3) {
        std::cerr << "Invalid args count\n";
        exit(1);
    }
    int ifGPU = atoi(argv[1]);
    if (ifGPU != 0 && ifGPU != 1) {
        std::cerr << "Invalid args\n";
        exit(1);
    }
    int fishCount = atoi(argv[2]);
    if (fishCount < 0 || fishCount > MAXFISHCOUNT) {
        std::cerr << "Invalid args\n";
        exit(1);
    }
    mainWindow w(fishCount, ifGPU);
    w.show();
    return app.exec();
}
