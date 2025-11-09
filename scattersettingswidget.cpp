#include "scattersettingswidget.h"
#include "./ui_scattersettingswidget.h"
#include "ComboItemDelegate.h"
#include "ColorDialogItemDelegate.h"
#include <QTableWidget>

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

void ScatterSettingsWidget::setupDelegates(QWidget* parent)
{
    // Столбец типа точки
    ComboItemDelegate* pointTypeDelegate = new ComboItemDelegate(parent);
    pointTypeDelegate->addItem("Круг", "circle");
    pointTypeDelegate->addItem("Квадрат", "square");
    pointTypeDelegate->addItem("Крестик", "cross");
    pointTypeDelegate->addItem("Плюс", "plus");
    pointTypeDelegate->addItem("Ромб", "diamond");
    ui->settingsTable->setItemDelegateForColumn(ColumnPointType, pointTypeDelegate);
    
    // Столбец цвета
    ColorDialogItemDelegate* colorDelegate = new ColorDialogItemDelegate(parent);
    ui->settingsTable->setItemDelegateForColumn(ColumnColor, colorDelegate);
}

