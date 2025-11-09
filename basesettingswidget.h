#ifndef BASESETTINGSWIDGET_H
#define BASESETTINGSWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QTableWidget;
QT_END_NAMESPACE

class BaseSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    enum CommonColumn {
        ColumnEnabled = 0
    };

    explicit BaseSettingsWidget(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual ~BaseSettingsWidget() = default;

    virtual QTableWidget* settingsTable() const = 0;
    virtual void setupDelegates(QWidget* parent) = 0;

    static BaseSettingsWidget* create(const QString& plotType, QWidget* parent = nullptr);
};

#endif // BASESETTINGSWIDGET_H

