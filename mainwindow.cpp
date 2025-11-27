#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "TableModel.h"
#include "InstrumentsModel.h"
#include "Experiment.h"
#include "Variable.h"
#include "AbsoluteInstrument.h"
#include "RelativeInstrument.h"
#include "CombinedInstrument.h"
#include "ComboItemDelegate.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tableModel(new TableModel(this))
    , m_instrumentsModel(new InstrumentsModel(this))
    , m_instrumentDelegate(new ComboItemDelegate(this))
    , m_errorTypeDelegate(new ComboItemDelegate(this))
    , m_noInstrument(std::make_shared<AbsoluteInstrument>("(нет инструмента)", 0.0))
{
    ui->setupUi(this);

    setupNoInstrument();
    createTestData();

    // Настраиваем таблицу измерений
    ui->tableViewMeasurements->setModel(m_tableModel);
    ui->tableViewMeasurements->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Настраиваем таблицу инструментов
    ui->tableViewInstruments->setModel(m_instrumentsModel);
    ui->tableViewInstruments->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_instrumentsModel->setInstruments(&m_instruments);

    // Настраиваем делегаты
    updateInstrumentDelegate();
    setupErrorTypeDelegate();

    // Применяем делегаты
    ui->variableInstrumentsTable->setItemDelegateForRow(0, m_instrumentDelegate);
    ui->tableViewInstruments->setItemDelegateForColumn(1, m_errorTypeDelegate);

    // Подключаем сигналы
    connect(ui->addColumnButton, &QPushButton::clicked, this, &MainWindow::addColumn);
    connect(ui->removeColumnButton, &QPushButton::clicked, this, &MainWindow::removeColumn);
    connect(ui->addRowButton, &QPushButton::clicked, this, &MainWindow::addRow);
    connect(ui->removeRowButton, &QPushButton::clicked, this, &MainWindow::removeRow);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::addInstrument);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::removeInstrument);
    connect(ui->variableInstrumentsTable, &QTableWidget::itemChanged, this, &MainWindow::onInstrumentChanged);

    connect(m_instrumentsModel, &InstrumentsModel::instrumentNameChanged, this, &MainWindow::updateInstrumentDelegate);
}

void MainWindow::setupNoInstrument()
{
    m_instruments.insert(m_instruments.begin(), m_noInstrument);
}

void MainWindow::updateInstrumentDelegate()
{
    if (m_instrumentDelegate) {
        delete m_instrumentDelegate;
    }
    m_instrumentDelegate = new ComboItemDelegate(this);

    m_instrumentDelegate->addItem("(нет инструмента)", "(нет инструмента)");

    for (size_t i = 1; i < m_instruments.size(); ++i) {
        const auto& instrument = m_instruments[i];
        QString name = QString::fromStdString(instrument->get_name());
        m_instrumentDelegate->addItem(name, name);
    }

    ui->variableInstrumentsTable->setItemDelegateForRow(0, m_instrumentDelegate);
}

void MainWindow::setupErrorTypeDelegate()
{
    m_errorTypeDelegate->addItem("Абсолютная", "Абсолютная");
    m_errorTypeDelegate->addItem("Относительная", "Относительная");
}

void MainWindow::onInstrumentChanged(QTableWidgetItem *item)
{
    if (!item) return;

    int column = item->column();
    QString selectedInstrument = item->text();

    auto experiment = Experiment::get_instance();

    if (column >= static_cast<int>(experiment->get_variables_count())) {
        return;
    }

    auto& var = experiment->get_variable(column);

    if (selectedInstrument == "(нет инструмента)") {
        var.add_instrument(m_noInstrument.get());
    } else {
        bool found = false;
        for (const auto& instrument : m_instruments) {
            if (instrument && QString::fromStdString(instrument->get_name()) == selectedInstrument) {
                var.add_instrument(instrument.get());
                found = true;
                break;
            }
        }

        if (!found) {
            var.add_instrument(m_noInstrument.get());
            item->setText("(нет инструмента)");
        }
    }

    m_tableModel->refreshData();
}

void MainWindow::createTestData()
{
    auto thermometer = std::make_shared<AbsoluteInstrument>("Термометр", 0.5);
    auto voltmeter = std::make_shared<RelativeInstrument>("Вольтметр", 0.02);

    m_instruments.push_back(thermometer);
    m_instruments.push_back(voltmeter);

    std::vector<double> temperatureData = {20.5, 21.2, 22.8, 23.1, 24.5, 25.0};
    std::vector<double> voltageData = {12.1, 12.3, 11.9, 12.5, 12.2, 12.4};
    std::vector<double> currentData = {1.5, 1.6, 1.55, 1.62, 1.58, 1.61};

    auto experiment = Experiment::get_instance();

    Variable tempVar(temperatureData, "Температура", "T", thermometer.get());
    Variable voltageVar(voltageData, "Напряжение", "U", voltmeter.get());
    Variable currentVar(currentData, "Ток", "I", m_noInstrument.get());

    experiment->add_variable(tempVar);
    experiment->add_variable(voltageVar);
    experiment->add_variable(currentVar);

    updateVariableInstrumentsTable();
    m_tableModel->refreshData();
}

void MainWindow::updateVariableInstrumentsTable()
{
    auto experiment = Experiment::get_instance();

    ui->variableInstrumentsTable->clear();
    ui->variableInstrumentsTable->setRowCount(1);
    ui->variableInstrumentsTable->setColumnCount(static_cast<int>(experiment->get_variables_count()));

    QStringList headers;
    for (size_t i = 0; i < experiment->get_variables_count(); ++i) {
        auto& var = experiment->get_variable(i);
        QString headerName = QString::fromStdString(var.get_name_tables());
        QString tag = QString::fromStdString(var.get_name_calculated());

        if (!tag.isEmpty()) {
            headerName += "\n(" + tag + ")";
        }
        headers << headerName;
    }
    ui->variableInstrumentsTable->setHorizontalHeaderLabels(headers);

    for (size_t i = 0; i < experiment->get_variables_count(); ++i) {
        auto& var = experiment->get_variable(i);

        QTableWidgetItem* item = new QTableWidgetItem();
        QString instrumentName = QString::fromStdString(var.get_name_instrument());
        item->setText(instrumentName);

        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->variableInstrumentsTable->setItem(0, static_cast<int>(i), item);
    }
}

void MainWindow::addColumn()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Добавить переменную", "Введите название переменной:", QLineEdit::Normal, "Переменная", &ok);
    if (!ok || name.isEmpty()) return;

    auto experiment = Experiment::get_instance();

    std::vector<double> measurements(6, 0.0);
    Variable newVar(measurements, name.toStdString(), "", m_noInstrument.get());
    experiment->add_variable(newVar);

    m_tableModel->refreshData();
    updateVariableInstrumentsTable();
}

void MainWindow::removeColumn()
{
    auto experiment = Experiment::get_instance();

    if (experiment->get_variables_count() == 0) {
        QMessageBox::information(this, "Информация", "Нет переменных для удаления");
        return;
    }

    experiment->remove_variable(experiment->get_variables_count() - 1);
    m_tableModel->refreshData();
    updateVariableInstrumentsTable();
}

void MainWindow::addRow()
{
    auto experiment = Experiment::get_instance();

    if (experiment->get_variables_count() == 0) {
        QMessageBox::information(this, "Информация", "Сначала добавьте переменные");
        return;
    }

    for (size_t i = 0; i < experiment->get_variables_count(); ++i) {
        auto& var = experiment->get_variable(i);
        var.add_measurement(0.0);
    }
    m_tableModel->refreshData();
}

void MainWindow::removeRow()
{
    auto experiment = Experiment::get_instance();

    if (experiment->get_variables_count() == 0) {
        QMessageBox::information(this, "Информация", "Нет данных для удаления");
        return;
    }

    for (size_t i = 0; i < experiment->get_variables_count(); ++i) {
        auto& var = experiment->get_variable(i);
        if (var.get_measurements_count() > 0) {
            var.remove_measurement(var.get_measurements_count() - 1);
        }
    }
    m_tableModel->refreshData();
}

void MainWindow::addInstrument()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Название инструмента", "Введите название инструмента:", QLineEdit::Normal, "Новый прибор", &ok);
    if (!ok || name.isEmpty()) return;

    std::shared_ptr<Instrument> instrument = std::make_shared<AbsoluteInstrument>(name.toStdString(), 0.0);

    m_instruments.push_back(instrument);
    m_instrumentsModel->refreshData();
    updateInstrumentDelegate();
}

void MainWindow::removeInstrument()
{
    if (m_instruments.size() <= 1) {
        QMessageBox::information(this, "Информация", "Нельзя удалить базовый инструмент 'Нет инструмента'");
        return;
    }

    auto experiment = Experiment::get_instance();
    auto instrumentToRemove = m_instruments.back();
    QString instrumentName = QString::fromStdString(instrumentToRemove->get_name());

    for (size_t i = 0; i < experiment->get_variables_count(); ++i) {
        auto& var = experiment->get_variable(i);
        if (QString::fromStdString(var.get_name_instrument()) == instrumentName) {
            var.add_instrument(m_noInstrument.get());
            QTableWidgetItem* item = ui->variableInstrumentsTable->item(0, static_cast<int>(i));
            if (item) {
                item->setText("(нет инструмента)");
            }
        }
    }

    m_instruments.pop_back();
    m_instrumentsModel->refreshData();
    updateInstrumentDelegate();
    m_tableModel->refreshData();
}

MainWindow::~MainWindow()
{
    delete ui;
}
