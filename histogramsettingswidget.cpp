#include "histogramsettingswidget.h"
#include "./ui_histogramsettingswidget.h"
#include "ComboItemDelegate.h"
#include "ColorDialogItemDelegate.h"
#include "OpacitySpinBoxDelegate.h"
#include "DoubleSpinBoxDelegate.h"
#include <QTableWidget>
#include <QComboBox>
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

void HistogramSettingsWidget::setupDelegates(QWidget* parent)
{
    // Столбец ширины столбца (используем DoubleSpinBoxDelegate для ввода произвольного значения)
    DoubleSpinBoxDelegate* widthDelegate = new DoubleSpinBoxDelegate(parent);
    ui->settingsTable->setItemDelegateForColumn(ColumnInterval, widthDelegate);
    
    // Столбец цвета
    ColorDialogItemDelegate* colorDelegate = new ColorDialogItemDelegate(parent);
    ui->settingsTable->setItemDelegateForColumn(ColumnColor, colorDelegate);
    
    // Столбец прозрачности
    OpacitySpinBoxDelegate* opacityDelegate = new OpacitySpinBoxDelegate(parent);
    ui->settingsTable->setItemDelegateForColumn(ColumnOpacity, opacityDelegate);
}

QComboBox* HistogramSettingsWidget::xAxisComboBox() const
{
    return ui->comboBox_xAxis;
}

QLineEdit* HistogramSettingsWidget::xAxisLabelEdit() const
{
    return ui->lineEdit_xAxisLabel;
}

QLineEdit* HistogramSettingsWidget::yAxisLabelEdit() const
{
    return ui->lineEdit_yAxisLabel;
}

