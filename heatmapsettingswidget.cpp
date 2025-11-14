#include "ComboItemDelegate.h"
#include "ColorDialogItemDelegate.h"
#include "heatmapsettingswidget.h"
#include "ui_heatmapsettingswidget.h"
#include <QTableWidget>
#include <QComboBox>
#include <QStringList>

HeatmapSettingsWidget::HeatmapSettingsWidget(QWidget *parent)
    : BaseSettingsWidget(parent)
    , ui(new Ui::HeatmapSettingsWidget)
{
    ui->setupUi(this);
    
    // Подключаем сигналы изменения выбора в ComboBox
    connect(ui->comboBox_xAxisLabel, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &HeatmapSettingsWidget::onAxisSelectionChanged);
    connect(ui->comboBox_yAxisLabel, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &HeatmapSettingsWidget::onAxisSelectionChanged);
}

HeatmapSettingsWidget::~HeatmapSettingsWidget()
{
    delete ui;
}

QTableWidget* HeatmapSettingsWidget::settingsTable() const
{
    return ui->settingsTable;
}

void HeatmapSettingsWidget::setupDelegates(QWidget* parent)
{
    // Столбец интервала
    ComboItemDelegate* intervalDelegate = new ComboItemDelegate(parent);
    intervalDelegate->addItem("Автоматически", "auto");
    intervalDelegate->addItem("10", "10");
    intervalDelegate->addItem("20", "20");
    intervalDelegate->addItem("50", "50");
    intervalDelegate->addItem("100", "100");
    ui->settingsTable->setItemDelegateForColumn(ColumnInterval, intervalDelegate);

    // Столбец цвета
    ColorDialogItemDelegate* colorDelegate = new ColorDialogItemDelegate(parent);
    ui->settingsTable->setItemDelegateForColumn(ColumnColor, colorDelegate);
}

void HeatmapSettingsWidget::updateColumnList(const QStringList& columnNames)
{
    // Сохраняем текущие выбранные значения
    QString currentXAxis = ui->comboBox_xAxisLabel->currentText();
    QString currentYAxis = ui->comboBox_yAxisLabel->currentText();
    
    // Сохраняем список колонок
    m_columnNames = columnNames;
    
    // Блокируем сигналы во время обновления
    ui->comboBox_xAxisLabel->blockSignals(true);
    ui->comboBox_yAxisLabel->blockSignals(true);
    
    // Очищаем ComboBox
    ui->comboBox_xAxisLabel->clear();
    ui->comboBox_yAxisLabel->clear();
    
    // Заполняем новыми значениями
    ui->comboBox_xAxisLabel->addItems(columnNames);
    ui->comboBox_yAxisLabel->addItems(columnNames);
    
    // Восстанавливаем выбранные значения, если они есть в новом списке
    int xIndex = ui->comboBox_xAxisLabel->findText(currentXAxis);
    if (xIndex >= 0) {
        ui->comboBox_xAxisLabel->setCurrentIndex(xIndex);
    } else if (!columnNames.isEmpty()) {
        // Если старое значение не найдено, выбираем первый элемент
        ui->comboBox_xAxisLabel->setCurrentIndex(0);
    }
    
    int yIndex = ui->comboBox_yAxisLabel->findText(currentYAxis);
    if (yIndex >= 0) {
        ui->comboBox_yAxisLabel->setCurrentIndex(yIndex);
    } else if (!columnNames.isEmpty()) {
        // Если старое значение не найдено, выбираем последний элемент (или первый, если всего один)
        int defaultYIndex = columnNames.size() > 1 ? columnNames.size() - 1 : 0;
        ui->comboBox_yAxisLabel->setCurrentIndex(defaultYIndex);
    }
    
    // Разблокируем сигналы
    ui->comboBox_xAxisLabel->blockSignals(false);
    ui->comboBox_yAxisLabel->blockSignals(false);
}

QString HeatmapSettingsWidget::getXAxisLabel() const
{
    return ui->comboBox_xAxisLabel->currentText();
}

QString HeatmapSettingsWidget::getYAxisLabel() const
{
    return ui->comboBox_yAxisLabel->currentText();
}

int HeatmapSettingsWidget::getXAxisColumnIndex() const
{
    int index = ui->comboBox_xAxisLabel->currentIndex();
    if (index >= 0 && index < m_columnNames.size()) {
        return index;
    }
    return -1;
}

int HeatmapSettingsWidget::getYAxisColumnIndex() const
{
    int index = ui->comboBox_yAxisLabel->currentIndex();
    if (index >= 0 && index < m_columnNames.size()) {
        return index;
    }
    return -1;
}

void HeatmapSettingsWidget::onAxisSelectionChanged()
{
    emit axisSelectionChanged();
}
