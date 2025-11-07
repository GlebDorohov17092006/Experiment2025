#include "scattersettingswidget.h"
#include "./ui_scattersettingswidget.h"
#include <QTableWidget>
#include <QLineEdit>

ScatterSettingsWidget::ScatterSettingsWidget(QWidget *parent)
    : BaseSettingsWidget(parent)
    , ui(new Ui::ScatterSettingsWidget)
{
    ui->setupUi(this);
}

ScatterSettingsWidget::~ScatterSettingsWidget()
{
    delete ui;
}

QTableWidget* ScatterSettingsWidget::settingsTable() const
{
    return ui->settingsTable;
}

QLineEdit* ScatterSettingsWidget::xAxisLabel() const
{
    return ui->lineEdit_xAxisLabel;
}

QLineEdit* ScatterSettingsWidget::yAxisLabel() const
{
    return ui->lineEdit_yAxisLabel;
}

