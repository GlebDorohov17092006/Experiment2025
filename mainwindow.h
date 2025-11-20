#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <memory>

class TableModel;
class InstrumentsModel;
class Experiment;
class Instrument;
class Variable;
class ComboItemDelegate;

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
    void onInstrumentChanged(QTableWidgetItem *item);

private:
    Ui::MainWindow *ui;
    TableModel* m_tableModel;
    InstrumentsModel* m_instrumentsModel;
    ComboItemDelegate* m_instrumentDelegate;
    ComboItemDelegate* m_errorTypeDelegate;
    std::vector<std::shared_ptr<Instrument>> m_instruments;
    Experiment* m_experiment;
    std::shared_ptr<Instrument> m_noInstrument; // Постоянный инструмент "Нет инструмента"

    void createTestData();
    void updateVariableInstrumentsTable();
    void updateInstrumentDelegate();
    void setupErrorTypeDelegate();
    void setupNoInstrument();
};
#endif // MAINWINDOW_H
