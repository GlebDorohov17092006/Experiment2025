#ifndef BASESETTINGSWIDGET_H
#define BASESETTINGSWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QTableWidget;
class QLineEdit;
QT_END_NAMESPACE

class BaseSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BaseSettingsWidget(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual ~BaseSettingsWidget() = default;

    virtual QTableWidget* settingsTable() const = 0;
    virtual QLineEdit* xAxisLabel() const = 0;
    virtual QLineEdit* yAxisLabel() const = 0;

    static BaseSettingsWidget* create(const QString& plotType, QWidget* parent = nullptr);
};

#endif // BASESETTINGSWIDGET_H

