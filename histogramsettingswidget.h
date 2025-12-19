#ifndef HISTOGRAMSETTINGSWIDGET_H
#define HISTOGRAMSETTINGSWIDGET_H

#include "basesettingswidget.h"

QT_BEGIN_NAMESPACE
class QTableWidget;
class QWidget;
class QComboBox;
class QLineEdit;
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
        ColumnOpacity = 3,
        ColumnCount = 4
    };

    explicit HistogramSettingsWidget(QWidget *parent = nullptr);
    ~HistogramSettingsWidget();

    QTableWidget* settingsTable() const override;
    void setupDelegates(QWidget* parent) override;
    QComboBox* xAxisComboBox() const;
    QLineEdit* xAxisLabelEdit() const;
    QLineEdit* yAxisLabelEdit() const;

private:
    Ui::HistogramSettingsWidget *ui;
};

#endif // HISTOGRAMSETTINGSWIDGET_H

