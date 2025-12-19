#include "heatmapsettingswidget.h"
#include "./ui_heatmapsettingswidget.h"
#include <QTableWidget>

HeatmapSettingsWidget::HeatmapSettingsWidget(QWidget *parent)
    : BaseSettingsWidget(parent)
    , ui(new Ui::HeatmapSettingsWidget)
{
    ui->setupUi(this);
}

HeatmapSettingsWidget::~HeatmapSettingsWidget()
{
    delete ui;
}

QTableWidget* HeatmapSettingsWidget::settingsTable() const
{
    return ui->settingsTable;
}

void HeatmapSettingsWidget::setupDelegates(QWidget* parent)
{
    Q_UNUSED(parent);
    // Пока нет настроек для хитмапы
}

