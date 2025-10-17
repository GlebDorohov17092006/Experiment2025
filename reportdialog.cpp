#include "reportdialog.h"
#include "./ui_reportdialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QCheckBox>

ReportDialog::ReportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReportDialog)
{
    ui->setupUi(this);
    
    // Подключаем сигналы к слотам
    connect(ui->generateButton, &QPushButton::clicked, this, &ReportDialog::onGenerateReport);
    connect(ui->cancelButton, &QPushButton::clicked, this, &ReportDialog::onCancel);
    connect(ui->browseButton, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку для сохранения", 
                                                       QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        if (!dir.isEmpty()) {
            ui->outputPathEdit->setText(dir);
        }
    });
    
    // Устанавливаем путь по умолчанию
    ui->outputPathEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
}

ReportDialog::~ReportDialog()
{
    delete ui;
}

void ReportDialog::onGenerateReport()
{
    if (ui->reportTitleEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Пожалуйста, введите название отчета.");
        return;
    }

    if (ui->outputPathEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите папку для сохранения.");
        return;
    }

    accept();
}

void ReportDialog::onCancel()
{
    reject();
}
