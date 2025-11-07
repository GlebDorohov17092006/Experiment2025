#ifndef HISTOGRAMSETTINGSWIDGET_H
#define HISTOGRAMSETTINGSWIDGET_H

#include "basesettingswidget.h"

QT_BEGIN_NAMESPACE
class QTableWidget;
class QLineEdit;
namespace Ui {
class HistogramSettingsWidget;
}
QT_END_NAMESPACE

class HistogramSettingsWidget : public BaseSettingsWidget
{
    Q_OBJECT

public:
    explicit HistogramSettingsWidget(QWidget *parent = nullptr);
    ~HistogramSettingsWidget();

    QTableWidget* settingsTable() const;
    QLineEdit* xAxisLabel() const;
    QLineEdit* yAxisLabel() const;

private:
    Ui::HistogramSettingsWidget *ui;
};

#endif // HISTOGRAMSETTINGSWIDGET_H

