#include "mainwindow.h"
#include "parser.h"
#include <windows.h>

#include <QApplication>
#include "Experiment.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    std::vector<Variable> variables = parser("../../electricChain.csv", "../../errors_tools.json");
    Experiment *experiment = Experiment::get_instance(variables, {});

    w.show();
    return a.exec();
}
