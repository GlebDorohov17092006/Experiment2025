#ifndef BASESETTINGSWIDGET_H
#define BASESETTINGSWIDGET_H

#include <QWidget>
#include <QStringList>

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
    
    // Виртуальный метод для обновления списка колонок (по умолчанию ничего не делает)
    virtual void updateColumnList(const QStringList& columnNames) { Q_UNUSED(columnNames); }

    static BaseSettingsWidget* create(const QString& plotType, QWidget* parent = nullptr);
};

#endif // BASESETTINGSWIDGET_H

