#ifndef PLOTSETTINGSWIDGET_H
#define PLOTSETTINGSWIDGET_H

#include "basesettingswidget.h"

QT_BEGIN_NAMESPACE
class QTableWidget;
class QLineEdit;
namespace Ui {
class PlotSettingsWidget;
}
QT_END_NAMESPACE

class PlotSettingsWidget : public BaseSettingsWidget
{
    Q_OBJECT

public:
    explicit PlotSettingsWidget(QWidget *parent = nullptr);
    ~PlotSettingsWidget();

    QTableWidget* settingsTable() const;
    QLineEdit* xAxisLabel() const;
    QLineEdit* yAxisLabel() const;

private:
    Ui::PlotSettingsWidget *ui;
};

#endif // PLOTSETTINGSWIDGET_H

