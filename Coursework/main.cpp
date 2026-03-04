#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setStyle("Fusion");
    MainWindow mainWindow;

    return app.exec();
}
