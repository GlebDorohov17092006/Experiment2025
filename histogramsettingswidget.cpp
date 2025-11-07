#include "histogramsettingswidget.h"
#include "./ui_histogramsettingswidget.h"
#include <QTableWidget>
#include <QLineEdit>

HistogramSettingsWidget::HistogramSettingsWidget(QWidget *parent)
    : BaseSettingsWidget(parent)
    , ui(new Ui::HistogramSettingsWidget)
{
    ui->setupUi(this);
}

HistogramSettingsWidget::~HistogramSettingsWidget()
{
    delete ui;
}

QTableWidget* HistogramSettingsWidget::settingsTable() const
{
    return ui->settingsTable;
}

QLineEdit* HistogramSettingsWidget::xAxisLabel() const
{
    return ui->lineEdit_xAxisLabel;
}

QLineEdit* HistogramSettingsWidget::yAxisLabel() const
{
    return ui->lineEdit_yAxisLabel;
}

