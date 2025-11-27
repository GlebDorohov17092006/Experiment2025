#ifndef HISTOGRAMSETTINGSWIDGET_H
#define HISTOGRAMSETTINGSWIDGET_H

#include "basesettingswidget.h"

QT_BEGIN_NAMESPACE
class QTableWidget;
class QWidget;
namespace Ui {
class HistogramSettingsWidget;
}
QT_END_NAMESPACE

class HistogramSettingsWidget : public BaseSettingsWidget
{
    Q_OBJECT

public:
    enum HistogramSettingsColumn {
        ColumnEnabled = 0,
        ColumnInterval = 1,
        ColumnColor = 2,
        ColumnCount = 3
    };

    explicit HistogramSettingsWidget(QWidget *parent = nullptr);
    ~HistogramSettingsWidget();

    QTableWidget* settingsTable() const override;
    void setupDelegates(QWidget* parent) override;

private:
    Ui::HistogramSettingsWidget *ui;
};

#endif // HISTOGRAMSETTINGSWIDGET_H

