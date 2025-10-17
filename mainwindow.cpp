#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "qcustomplot.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->linePlot->setInteraction(QCP::iRangeDrag, true);
    ui->linePlot->setInteraction(QCP::iRangeZoom, true);

    connect(ui->tableWidget->horizontalHeader(), &QHeaderView::sectionDoubleClicked,
            this, [this](int col) {
                bool ok;

                QTableWidgetItem *headerItem = ui->tableWidget->horizontalHeaderItem(col);
                QString oldName;
                if (headerItem) {
                    oldName = headerItem->text();
                } else {
                    oldName = QString("Column %1").arg(col + 1);
                    ui->tableWidget->setHorizontalHeaderItem(col, new QTableWidgetItem(oldName));
                    headerItem = ui->tableWidget->horizontalHeaderItem(col);
                }

                QString newName = QInputDialog::getText(this,
                                                        "Переименование столбца",
                                                        "Новое имя:",
                                                        QLineEdit::Normal,
                                                        oldName,
                                                        &ok);
                if (ok && !newName.isEmpty()) {
                    ui->tableWidget->setHorizontalHeaderItem(col, new QTableWidgetItem(newName));
                }
            });

    ui->instrumentsTable->setHorizontalHeaderLabels({"Прибор", "Описание", "Погрешность"});
}

MainWindow::~MainWindow()
{
    delete ui;
}
