#include "basesettingswidget.h"
#include "scattersettingswidget.h"
#include "histogramsettingswidget.h"
#include "plotsettingswidget.h"
#include "heatmapsettingswidget.h"

BaseSettingsWidget* BaseSettingsWidget::create(const QString& plotType, QWidget* parent)
{
    if (plotType == "График") {
        return new PlotSettingsWidget(parent);
    } else if (plotType == "Гистограмма") {
        return new HistogramSettingsWidget(parent);
    } else if (plotType == "Скаттерплот") {
        return new ScatterSettingsWidget(parent);
    } else if (plotType == "Тепловая карта") {
        return new HeatmapSettingsWidget(parent);
    }
    return nullptr;
}

