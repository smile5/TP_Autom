#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.setWindowIcon(QIcon(":/automatic_conveyor-512.png"));
    w.setWindowTitle("Ho !! Tom !!!");
    return a.exec();
}
