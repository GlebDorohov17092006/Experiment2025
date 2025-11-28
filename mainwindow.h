#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <memory>

class TableModel;
class InstrumentsModel;
class Experiment;
class Instrument;
class Variable;
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
    void draw_line_plot(int first_in, int second_in);

private slots:
    void addColumn();
    void removeColumn();
    void addRow();
    void removeRow();
    void addInstrument();
    void removeInstrument();
    void onPlotSettingsTabChanged(int index);
    void onPlotTabChanged(int index);
    void onPlotTabMoved(int from, int to);
    void onColumnHeaderDoubleClicked(int col);
    void saveReport();
    void onInstrumentCellDoubleClicked(int row, int column);
    void removeGraph(int index = -1);
    void updateInstrumentTexts();
    void addTextBlockToReport();
    void addTableBlockToReport();
    void addPlotBlockToReport();

    void on_import_CSV_triggered();

private:
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
    struct PlotTab {
        QString name;
        QString type;
        QCustomPlot* plot;
        QWidget* settingsTab;
        QTableWidget* settingsTable;
    };

private:
    Ui::MainWindow *ui;
    TableModel* m_tableModel;
    InstrumentsModel* m_instrumentsModel;
    std::vector<std::shared_ptr<Instrument>> m_instruments;
    Experiment* m_experiment;

    // Добавляем объявления недостающих методов
    void createTestData();
    void updateVariableInstrumentsTable();
    QMap<int, QString> m_columnTags; // Хранилище тегов столбцов
    QList<PlotTab> m_plotTabs; // Список динамически добавленных графиков
    QList<ReportBlock*> m_reportBlocks; // Список блоков отчета
};
#endif // MAINWINDOW_H
