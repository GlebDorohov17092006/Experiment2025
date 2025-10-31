#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

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
    void drawSimpleGraph();
    void onPlotSettingsTabChanged(int index);
    void onPlotTabChanged(int index);
    void onColumnHeaderDoubleClicked(int col);
    void openReportDialog();
    void onInstrumentCellDoubleClicked(int row, int column);

    void on_import_CSV_triggered();

private:
    void setColumnTag(int columnIndex, const QString& tag);
    QString getColumnTag(int columnIndex);
    QString getColumnName(int columnIndex);
    void syncVariableInstrumentsTable();
    void syncPlotSettingsTables();
    void updateInstrumentTexts();
    QString getInstrumentDisplayText(int instrumentIndex);
    void addPlotSettingsRow(const QString& columnName);

private:
    Ui::MainWindow *ui;
    QMap<int, QString> m_columnTags; // Хранилище тегов столбцов
};
#endif // MAINWINDOW_H
