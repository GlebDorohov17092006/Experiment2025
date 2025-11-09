#include "histogramsettingswidget.h"
#include "./ui_histogramsettingswidget.h"
#include "ComboItemDelegate.h"
#include "ColorDialogItemDelegate.h"
#include <QTableWidget>

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
    // Столбец интервала
    ComboItemDelegate* intervalDelegate = new ComboItemDelegate(parent);
    intervalDelegate->addItem("Автоматически", "auto");
    intervalDelegate->addItem("10", "10");
    intervalDelegate->addItem("20", "20");
    intervalDelegate->addItem("50", "50");
    intervalDelegate->addItem("100", "100");
    ui->settingsTable->setItemDelegateForColumn(ColumnInterval, intervalDelegate);
    
    // Столбец цвета
    ColorDialogItemDelegate* colorDelegate = new ColorDialogItemDelegate(parent);
    ui->settingsTable->setItemDelegateForColumn(ColumnColor, colorDelegate);
}

