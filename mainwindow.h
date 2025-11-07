#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

class TableModel;
class InstrumentsModel;
class Experiment;
class Instrument;
class Variable;

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

private:
    Ui::MainWindow *ui;
    TableModel* m_tableModel;
    InstrumentsModel* m_instrumentsModel;
    std::vector<std::shared_ptr<Instrument>> m_instruments;
    Experiment* m_experiment;

    // Добавляем объявления недостающих методов
    void createTestData();
    void updateVariableInstrumentsTable();
};
#endif // MAINWINDOW_H
