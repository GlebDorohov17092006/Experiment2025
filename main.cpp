#include "mainwindow.h"
#include "parser.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    parser("electricChain.csv", "errors_tools.json");

    w.show();
    return a.exec();
}
