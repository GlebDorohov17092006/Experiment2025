#include "plotsettingswidget.h"
#include "./ui_plotsettingswidget.h"
#include <QTableWidget>
#include <QLineEdit>

PlotSettingsWidget::PlotSettingsWidget(QWidget *parent)
    : BaseSettingsWidget(parent)
    , ui(new Ui::PlotSettingsWidget)
{
    ui->setupUi(this);
}

PlotSettingsWidget::~PlotSettingsWidget()
{
    delete ui;
}

QTableWidget* PlotSettingsWidget::settingsTable() const
{
    return ui->settingsTable;
}

QLineEdit* PlotSettingsWidget::xAxisLabel() const
{
    return ui->lineEdit_xAxisLabel;
}

QLineEdit* PlotSettingsWidget::yAxisLabel() const
{
    return ui->lineEdit_yAxisLabel;
}

