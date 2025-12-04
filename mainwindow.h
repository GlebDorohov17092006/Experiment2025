#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QMap>
#include <memory>

class TableModel;
class InstrumentsModel;
class Experiment;
class Instrument;
class Variable;
class ComboItemDelegate;
class QCustomPlot;
class QCPGraph;
class QTableWidget;
class QFrame;
class ReportBlock;
class PlotSettingsWidget;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void draw_line_plot(int first_in, int second_in, int plot_tab_index);

private slots:
    // Основные слоты из обеих веток
    void addColumn();
    void removeColumn();
    void addRow();
    void removeRow();
    void addInstrument();
    void removeInstrument();
    void onInstrumentChanged(QTableWidgetItem *item);

    // Слоты из ветки main (дополнительный функционал)
    void onPlotSettingsTabChanged(int index);
    void onPlotTabChanged(int index);
    void onPlotTabMoved(int from, int to);
    void onColumnHeaderDoubleClicked(int col);
    void saveReport();
    void onInstrumentCellDoubleClicked(int row, int column);
    void removeGraph(int index = -1);
    void addTextBlockToReport();
    void addTableBlockToReport();
    void addPlotBlockToReport();
    void on_import_data_triggered();
    void on_export_data_triggered();

private:
    // Методы из ветки Gleb (основная логика)
    void updateVariableInstrumentsTable();
    void updateInstrumentDelegate();
    void setupErrorTypeDelegate();
    void setupNoInstrument();

    // Методы из ветки main (дополнительный функционал)
    void connectReportBlockDeletion(QFrame* frame, ReportBlock* block);
    void setColumnTag(int columnIndex, const QString& tag);
    QString getColumnTag(int columnIndex);
    QString getColumnName(int columnIndex);
    void syncVariableInstrumentsTable();
    void syncPlotSettingsTables();
    void applyPlotSettingsFromTable(QCPGraph* graph, QTableWidget* settingsTable, int rowIndex);
    void updateVariableComboBoxes(PlotSettingsWidget* plotSettings);
    QString getInstrumentDisplayText(int instrumentIndex);
    void addDynamicPlotTab(const QString& plotType);

    // Структура для хранения информации о графиках (из ветки main)
    struct PlotTab {
        QString name;
        QString type;
        QCustomPlot* plot;
        QWidget* settingsTab;
        QTableWidget* settingsTable;

        // Добавляем оператор сравнения как const
        bool operator==(const PlotTab& other) const {
            return name == other.name &&
                   type == other.type &&
                   plot == other.plot &&
                   settingsTab == other.settingsTab &&
                   settingsTable == other.settingsTable;
        }

        // Можно добавить и оператор != для полноты
        bool operator!=(const PlotTab& other) const {
            return !(*this == other);
        }
    };

private:
    Ui::MainWindow *ui;

    // Общие модели и данные
    TableModel* m_tableModel;
    InstrumentsModel* m_instrumentsModel;
    std::vector<std::shared_ptr<Instrument>> m_instruments;

    // из ветки Gleb
    ComboItemDelegate* m_instrumentDelegate;
    ComboItemDelegate* m_errorTypeDelegate;
    std::shared_ptr<Instrument> m_noInstrument;

    // Из ветки main
    QMap<int, QString> m_columnTags; // Хранилище тегов столбцов
    QList<PlotTab> m_plotTabs; // Список динамически добавленных графиков
    QList<ReportBlock*> m_reportBlocks; // Список блоков отчета
};

#endif // MAINWINDOW_H
