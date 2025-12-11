#include "plotsettingswidget.h"
#include "./ui_plotsettingswidget.h"
#include "ComboItemDelegate.h"
#include "ColorDialogItemDelegate.h"
#include <QTableWidget>
#include <QComboBox>
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

void PlotSettingsWidget::setupDelegates(QWidget* parent)
{
    // Столбец типа линии
    ComboItemDelegate* lineTypeDelegate = new ComboItemDelegate(parent);
    lineTypeDelegate->addItem("Сплошная", "line");
    lineTypeDelegate->addItem("Ступенчатая", "step");
    ui->settingsTable->setItemDelegateForColumn(ColumnLineType, lineTypeDelegate);
    
    // Столбец типа точки
    ComboItemDelegate* pointTypeDelegate = new ComboItemDelegate(parent);
    pointTypeDelegate->addItem("Круг", "circle");
    pointTypeDelegate->addItem("Квадрат", "square");
    pointTypeDelegate->addItem("Крестик", "cross");
    pointTypeDelegate->addItem("Плюс", "plus");
    pointTypeDelegate->addItem("Ромб", "diamond");
    pointTypeDelegate->addItem("Без точки", "none");
    ui->settingsTable->setItemDelegateForColumn(ColumnPointType, pointTypeDelegate);
    
    // Столбец цвета
    ColorDialogItemDelegate* colorDelegate = new ColorDialogItemDelegate(parent);
    ui->settingsTable->setItemDelegateForColumn(ColumnColor, colorDelegate);
}

QComboBox* PlotSettingsWidget::yAxisComboBox() const
{
    return ui->comboBox_yAxis;
}

QLineEdit* PlotSettingsWidget::xAxisLabelEdit() const
{
    return ui->lineEdit_xAxisLabel;
}

QLineEdit* PlotSettingsWidget::yAxisLabelEdit() const
{
    return ui->lineEdit_yAxisLabel;
}

