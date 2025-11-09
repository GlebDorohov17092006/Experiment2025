#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
class QCustomPlot;
class QTableWidget;

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
    void openReportDialog();
    void onInstrumentCellDoubleClicked(int row, int column);
    void removeGraph(int index = -1);
    void updateInstrumentTexts();

private:
    void setColumnTag(int columnIndex, const QString& tag);
    QString getColumnTag(int columnIndex);
    QString getColumnName(int columnIndex);
    void syncVariableInstrumentsTable();
    void syncPlotSettingsTables();
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
    QMap<int, QString> m_columnTags; // Хранилище тегов столбцов
    QList<PlotTab> m_plotTabs; // Список динамически добавленных графиков
};
#endif // MAINWINDOW_H
