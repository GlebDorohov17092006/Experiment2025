#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "TableModel.h"
#include "InstrumentsModel.h"
#include "Experiment.h"
#include "Variable.h"
#include "AbsoluteInstrument.h"
#include "RelativeInstrument.h"
#include "CombinedInstrument.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QTableView>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tableModel(new TableModel(this))
    , m_instrumentsModel(new InstrumentsModel(this))
    , m_experiment(nullptr)
{
    ui->setupUi(this);

    // Инициализируем эксперимент
    std::vector<Variable> variables;
    std::vector<Variable> calculated_variables;
    m_experiment = Experiment::get_instance(variables, calculated_variables);

    // СОЗДАЕМ ТЕСТОВЫЕ ДАННЫЕ
    createTestData();

    // Привязываем модели
    m_tableModel->setExperiment(m_experiment);
    m_instrumentsModel->setInstruments(&m_instruments);

    // ЗАМЕНЯЕМ tableWidget на TableView
    QTableView* measurementsView = new QTableView();
    measurementsView->setModel(m_tableModel);
    measurementsView->setSelectionBehavior(QAbstractItemView::SelectItems);
    measurementsView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    measurementsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Заменяем tableWidget в splitter_2
    for (int i = 0; i < ui->splitter_2->count(); ++i) {
        if (ui->splitter_2->widget(i) == ui->tableWidget) {
            ui->splitter_2->replaceWidget(i, measurementsView);
            ui->tableWidget->deleteLater();
            break;
        }
    }

    // ЗАМЕНЯЕМ instrumentsTable на TableView
    QTableView* instrumentsView = new QTableView();
    instrumentsView->setModel(m_instrumentsModel);
    instrumentsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    instrumentsView->verticalHeader()->setVisible(false);

    // Заменяем instrumentsTable в gridLayout_5
    QLayoutItem* item = ui->gridLayout_5->itemAtPosition(0, 0);
    if (item && item->widget() == ui->instrumentsTable) {
        ui->gridLayout_5->removeWidget(ui->instrumentsTable);
        ui->gridLayout_5->addWidget(instrumentsView, 0, 0, 1, 2);
        ui->instrumentsTable->deleteLater();
    }

    connect(ui->addColumnButton, &QPushButton::clicked, this, &MainWindow::addColumn);
    connect(ui->removeColumnButton, &QPushButton::clicked, this, &MainWindow::removeColumn);
    connect(ui->addRowButton, &QPushButton::clicked, this, &MainWindow::addRow);
    connect(ui->removeRowButton, &QPushButton::clicked, this, &MainWindow::removeRow);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::addInstrument);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::removeInstrument);
}

void MainWindow::createTestData()
{
    // СОЗДАЕМ ТЕСТОВЫЕ ИНСТРУМЕНТЫ
    auto thermometer = std::make_shared<AbsoluteInstrument>("Термометр", 0.5);
    auto voltmeter = std::make_shared<RelativeInstrument>("Вольтметр", 0.02); // 2%
    auto multimeter = std::make_shared<CombinedInstrument>("Мультиметр");
    auto multimeterPtr = std::dynamic_pointer_cast<CombinedInstrument>(multimeter);
    if (multimeterPtr) {
        multimeterPtr->add_error(0.1);
        multimeterPtr->add_error(0.05);
    }

    m_instruments.push_back(thermometer);
    m_instruments.push_back(voltmeter);
    m_instruments.push_back(multimeter);

    // СОЗДАЕМ ТЕСТОВЫЕ ПЕРЕМЕННЫЕ С ДАННЫМИ
    std::vector<double> temperatureData = {20.5, 21.2, 22.8, 23.1, 24.5, 25.0};
    std::vector<double> voltageData = {12.1, 12.3, 11.9, 12.5, 12.2, 12.4};
    std::vector<double> currentData = {1.5, 1.6, 1.55, 1.62, 1.58, 1.61};

    Variable tempVar(temperatureData, "Температура", "T", thermometer.get());
    Variable voltageVar(voltageData, "Напряжение", "U", voltmeter.get());
    Variable currentVar(currentData, "Ток", "I", multimeter.get());

    m_experiment->add_variable(tempVar);
    m_experiment->add_variable(voltageVar);
    m_experiment->add_variable(currentVar);

    // ОБНОВЛЯЕМ ТАБЛИЦУ ИНСТРУМЕНТОВ ПЕРЕМЕННЫХ
    updateVariableInstrumentsTable();
}

void MainWindow::updateVariableInstrumentsTable()
{
    // Очищаем таблицу
    ui->variableInstrumentsTable->clear();
    ui->variableInstrumentsTable->setRowCount(1);
    ui->variableInstrumentsTable->setColumnCount(m_experiment->get_variables_count());

    // Заполняем заголовки и данные об инструментах
    for (size_t i = 0; i < m_experiment->get_variables_count(); ++i) {
        auto var = m_experiment->get_variable(i);
        QString headerName = QString::fromStdString(var->get_name_tables());
        QString tag = QString::fromStdString(var->get_name_calculated());

        if (!tag.isEmpty()) {
            headerName += "\n(" + tag + ")";
        }

        ui->variableInstrumentsTable->setHorizontalHeaderItem(i, new QTableWidgetItem(headerName));

        // Заполняем инструмент
        QTableWidgetItem* item = new QTableWidgetItem();
        try {
            QString instrumentName = QString::fromStdString(var->get_name_instrument());
            item->setText(instrumentName);
        } catch (const std::exception&) {
            item->setText("(нет инструмента)");
        }
        ui->variableInstrumentsTable->setItem(0, i, item);
    }
}

void MainWindow::addColumn()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Добавить переменную",
                                        "Введите название переменной:",
                                        QLineEdit::Normal, "Переменная", &ok);
    if (!ok || name.isEmpty()) return;

    // Выбираем инструмент если есть
    Instrument* selectedInstrument = nullptr;
    if (!m_instruments.empty()) {
        QStringList instrumentNames;
        for (const auto& instr : m_instruments) {
            instrumentNames << QString::fromStdString(instr->get_name());
        }

        QString instrumentName = QInputDialog::getItem(this, "Выбор инструмента",
                                                      "Выберите инструмент:",
                                                      instrumentNames, 0, false, &ok);
        if (ok) {
            for (const auto& instr : m_instruments) {
                if (QString::fromStdString(instr->get_name()) == instrumentName) {
                    selectedInstrument = instr.get();
                    break;
                }
            }
        }
    }

    // Создаем переменную с начальными данными
    std::vector<double> measurements(6, 0.0); // 6 пустых значений
    Variable newVar(measurements, name.toStdString(), "", selectedInstrument);
    m_experiment->add_variable(newVar);
    m_tableModel->refreshData();

    // Обновляем таблицу инструментов переменных
    updateVariableInstrumentsTable();
}

void MainWindow::removeColumn()
{
    if (m_experiment->get_variables_count() == 0) {
        QMessageBox::information(this, "Информация", "Нет переменных для удаления");
        return;
    }

    m_experiment->remove_variable(m_experiment->get_variables_count() - 1);
    m_tableModel->refreshData();
    updateVariableInstrumentsTable();
}

void MainWindow::addRow()
{
    if (m_experiment->get_variables_count() == 0) {
        QMessageBox::information(this, "Информация", "Сначала добавьте переменные");
        return;
    }

    for (size_t i = 0; i < m_experiment->get_variables_count(); ++i) {
        auto var = m_experiment->get_variable(i);
        var->add_measurement(0.0);
    }
    m_tableModel->refreshData();
}

void MainWindow::removeRow()
{
    if (m_experiment->get_variables_count() == 0) {
        QMessageBox::information(this, "Информация", "Нет данных для удаления");
        return;
    }

    for (size_t i = 0; i < m_experiment->get_variables_count(); ++i) {
        auto var = m_experiment->get_variable(i);
        if (var->get_measurements_count() > 0) {
            var->remove_measurement(var->get_measurements_count() - 1);
        }
    }
    m_tableModel->refreshData();
}

void MainWindow::addInstrument()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Название инструмента",
                                        "Введите название инструмента:",
                                        QLineEdit::Normal, "Новый прибор", &ok);
    if (!ok || name.isEmpty()) return;

    QStringList types = {"Абсолютная", "Относительная", "Комбинированная"};
    QString type = QInputDialog::getItem(this, "Тип инструмента",
                                        "Выберите тип погрешности:",
                                        types, 0, false, &ok);
    if (!ok) return;

    double error = QInputDialog::getDouble(this, "Погрешность",
                                          "Введите значение погрешности:",
                                          0.1, 0, 100, 3, &ok);
    if (!ok) return;

    std::shared_ptr<Instrument> instrument;

    if (type == "Абсолютная") {
        instrument = std::make_shared<AbsoluteInstrument>(name.toStdString(), error);
    } else if (type == "Относительная") {
        instrument = std::make_shared<RelativeInstrument>(name.toStdString(), error);
    } else {
        instrument = std::make_shared<CombinedInstrument>(name.toStdString());
        auto combPtr = std::dynamic_pointer_cast<CombinedInstrument>(instrument);
        if (combPtr) {
            combPtr->add_error(error);
        }
    }

    m_instruments.push_back(instrument);
    m_instrumentsModel->refreshData();
}

void MainWindow::removeInstrument()
{
    if (m_instruments.empty()) {
        QMessageBox::information(this, "Информация", "Нет приборов для удаления");
        return;
    }

    m_instruments.pop_back();
    m_instrumentsModel->refreshData();
}

MainWindow::~MainWindow()
{
    delete ui;
}
