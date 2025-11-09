#ifndef SCATTERSETTINGSWIDGET_H
#define SCATTERSETTINGSWIDGET_H

#include "basesettingswidget.h"

QT_BEGIN_NAMESPACE
class QTableWidget;
class QWidget;
namespace Ui {
class ScatterSettingsWidget;
}
QT_END_NAMESPACE

class ScatterSettingsWidget : public BaseSettingsWidget
{
    Q_OBJECT

public:
    enum ScatterSettingsColumn {
        ColumnEnabled = 0,
        ColumnPointSize = 1,
        ColumnPointType = 2,
        ColumnColor = 3,
        ColumnCount = 4
    };

    explicit ScatterSettingsWidget(QWidget *parent = nullptr);
    ~ScatterSettingsWidget();

    QTableWidget* settingsTable() const override;
    void setupDelegates(QWidget* parent) override;

private:
    Ui::ScatterSettingsWidget *ui;
};

#endif // SCATTERSETTINGSWIDGET_H

