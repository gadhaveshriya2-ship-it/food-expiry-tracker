#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Smart Food Expiry Tracker");
    QApplication::setOrganizationName("Smart Food Expiry Tracker");
    MainWindow window;
    window.show();
    return app.exec();
}
