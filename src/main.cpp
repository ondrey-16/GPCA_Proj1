//
// Created by ondrey on 31.10.2025.
//

#include <QApplication>
#include "gui/mainWindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    mainWindow w;
    w.show();
    return app.exec();
}
