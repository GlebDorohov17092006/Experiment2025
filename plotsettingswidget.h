#ifndef PLOTSETTINGSWIDGET_H
#define PLOTSETTINGSWIDGET_H

#include "basesettingswidget.h"

QT_BEGIN_NAMESPACE
class QTableWidget;
class QWidget;
namespace Ui {
class PlotSettingsWidget;
}
QT_END_NAMESPACE

class PlotSettingsWidget : public BaseSettingsWidget
{
    Q_OBJECT

public:
    enum PlotSettingsColumn {
        ColumnEnabled = 0,
        ColumnLineType = 1,
        ColumnWidth = 2,
        ColumnPointType = 3,
        ColumnPointSize = 4,
        ColumnColor = 5,
        ColumnCount = 6
    };

    explicit PlotSettingsWidget(QWidget *parent = nullptr);
    ~PlotSettingsWidget();

    QTableWidget* settingsTable() const override;
    void setupDelegates(QWidget* parent) override;

private:
    Ui::PlotSettingsWidget *ui;
};

#endif // PLOTSETTINGSWIDGET_H

