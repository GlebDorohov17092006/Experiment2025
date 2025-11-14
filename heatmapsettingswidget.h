#ifndef HEATMAPSETTINGSWIDGET_H
#define HEATMAPSETTINGSWIDGET_H

#include "basesettingswidget.h"

QT_BEGIN_NAMESPACE
class QTableWidget;
class QWidget;
namespace Ui {
class HeatmapSettingsWidget;
}
QT_END_NAMESPACE

class HeatmapSettingsWidget : public BaseSettingsWidget
{
    Q_OBJECT

public:
    enum HistogramSettingsColumn {
        ColumnEnabled = 0,
        ColumnInterval = 1,
        ColumnColor = 2,
        ColumnCount = 3
    };

    explicit HeatmapSettingsWidget(QWidget *parent = nullptr);
    ~HeatmapSettingsWidget();

    QTableWidget* settingsTable() const override;
    void setupDelegates(QWidget* parent) override;
    
    // Методы для работы с ComboBox осей
    void updateColumnList(const QStringList& columnNames) override;
    QString getXAxisLabel() const;
    QString getYAxisLabel() const;
    int getXAxisColumnIndex() const;
    int getYAxisColumnIndex() const;

signals:
    void axisSelectionChanged(); // Сигнал при изменении выбора осей

private slots:
    void onAxisSelectionChanged();

private:
    Ui::HeatmapSettingsWidget *ui;
    QStringList m_columnNames; // Храним список колонок для получения индексов
};

#endif // HEATMAPSETTINGSWIDGET_H
