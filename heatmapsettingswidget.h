#ifndef HEATMAPSETTINGSWIDGET_H
#define HEATMAPSETTINGSWIDGET_H

#include "basesettingswidget.h"

QT_BEGIN_NAMESPACE
class QTableWidget;
class QWidget;
namespace Ui {
class HeatmapSettingsWidget;
}
QT_END_NAMESPACE

class HeatmapSettingsWidget : public BaseSettingsWidget
{
    Q_OBJECT

public:
    explicit HeatmapSettingsWidget(QWidget *parent = nullptr);
    ~HeatmapSettingsWidget();

    QTableWidget* settingsTable() const override;
    void setupDelegates(QWidget* parent) override;

private:
    Ui::HeatmapSettingsWidget *ui;
};

#endif // HEATMAPSETTINGSWIDGET_H

