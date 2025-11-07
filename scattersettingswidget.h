#ifndef SCATTERSETTINGSWIDGET_H
#define SCATTERSETTINGSWIDGET_H

#include "basesettingswidget.h"

QT_BEGIN_NAMESPACE
class QTableWidget;
class QLineEdit;
namespace Ui {
class ScatterSettingsWidget;
}
QT_END_NAMESPACE

class ScatterSettingsWidget : public BaseSettingsWidget
{
    Q_OBJECT

public:
    explicit ScatterSettingsWidget(QWidget *parent = nullptr);
    ~ScatterSettingsWidget();

    QTableWidget* settingsTable() const;
    QLineEdit* xAxisLabel() const;
    QLineEdit* yAxisLabel() const;

private:
    Ui::ScatterSettingsWidget *ui;
};

#endif // SCATTERSETTINGSWIDGET_H

