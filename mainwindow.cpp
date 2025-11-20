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
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tableModel(new TableModel(this))
    , m_instrumentsModel(new InstrumentsModel(this))
    , m_instrumentDelegate(new ComboItemDelegate(this))
    , m_errorTypeDelegate(new ComboItemDelegate(this))
    , m_experiment(Experiment::get_instance())
    , m_noInstrument(std::make_shared<AbsoluteInstrument>("(нет инструмента)", 0.0))
{
    ui->setupUi(this);

    setupNoInstrument();
    createTestData();

    // Заменяем QTableWidget на QTableView для измерений
    QTableView* measurementsView = new QTableView();
    measurementsView->setModel(m_tableModel);
    measurementsView->setSelectionBehavior(QAbstractItemView::SelectItems);
    measurementsView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    measurementsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int i = 0; i < ui->splitter_2->count(); ++i) {
        if (ui->splitter_2->widget(i) == ui->tableWidget) {
            ui->splitter_2->replaceWidget(i, measurementsView);
            ui->tableWidget->deleteLater();
            break;
        }
    }

    // Заменяем QTableWidget на QTableView для инструментов
    QTableView* instrumentsView = new QTableView();
    instrumentsView->setModel(m_instrumentsModel);
    instrumentsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    instrumentsView->verticalHeader()->setVisible(false);

    QLayoutItem* item = ui->gridLayout_5->itemAtPosition(0, 0);
    if (item && item->widget() == ui->instrumentsTable) {
        ui->gridLayout_5->removeWidget(ui->instrumentsTable);
        ui->gridLayout_5->addWidget(instrumentsView, 0, 0, 1, 2);
        ui->instrumentsTable->deleteLater();
    }

    m_instrumentsModel->setInstruments(&m_instruments);

    // Настраиваем делегаты
    updateInstrumentDelegate();
    setupErrorTypeDelegate();

    // Применяем делегаты к таблицам
    ui->variableInstrumentsTable->setItemDelegateForRow(0, m_instrumentDelegate);
    instrumentsView->setItemDelegateForColumn(1, m_errorTypeDelegate);

    // Подключаем сигналы
    connect(ui->addColumnButton, &QPushButton::clicked, this, &MainWindow::addColumn);
    connect(ui->removeColumnButton, &QPushButton::clicked, this, &MainWindow::removeColumn);
    connect(ui->addRowButton, &QPushButton::clicked, this, &MainWindow::addRow);
    connect(ui->removeRowButton, &QPushButton::clicked, this, &MainWindow::removeRow);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::addInstrument);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::removeInstrument);
    connect(ui->variableInstrumentsTable, &QTableWidget::itemChanged,
            this, &MainWindow::onInstrumentChanged);

    // Подключаем сигнал изменения названия инструмента
    connect(m_instrumentsModel, &InstrumentsModel::instrumentNameChanged,
            this, &MainWindow::updateInstrumentDelegate);
}

void MainWindow::setupNoInstrument()
{
    // Добавляем постоянный инструмент "Нет инструмента" в начало списка
    m_instruments.insert(m_instruments.begin(), m_noInstrument);
}

void MainWindow::updateInstrumentDelegate()
{
    // Создаем новый делегат вместо пересоздания
    if (m_instrumentDelegate) {
        delete m_instrumentDelegate;
    }
    m_instrumentDelegate = new ComboItemDelegate(this);

    // Добавляем опцию "Нет инструмента"
    m_instrumentDelegate->addItem("(нет инструмента)", "(нет инструмента)");

    // Добавляем все доступные инструменты (кроме самого "Нет инструмента")
    for (size_t i = 1; i < m_instruments.size(); ++i) {
        const auto& instrument = m_instruments[i];
        QString name = QString::fromStdString(instrument->get_name());
        m_instrumentDelegate->addItem(name, name);
    }

    // Применяем обновленный делегат
    ui->variableInstrumentsTable->setItemDelegateForRow(0, m_instrumentDelegate);
}

void MainWindow::setupErrorTypeDelegate()
{
    // Делегат для выбора типа погрешности
    m_errorTypeDelegate->addItem("Абсолютная", "Абсолютная");
    m_errorTypeDelegate->addItem("Относительная", "Относительная");
}

void MainWindow::onInstrumentChanged(QTableWidgetItem *item)
{
    if (!item) return;

    int column = item->column();
    QString selectedInstrument = item->text();

    if (column >= static_cast<int>(m_experiment->get_variables_count())) {
        return;
    }

    auto var = m_experiment->get_variable(column);

    if (selectedInstrument == "(нет инструмента)") {
        var->add_instrument(m_noInstrument.get()); // Используем постоянный инструмент
    } else {
        // Находим выбранный инструмент
        bool found = false;
        for (const auto& instrument : m_instruments) {
            if (QString::fromStdString(instrument->get_name()) == selectedInstrument) {
                var->add_instrument(instrument.get());
                found = true;
                break;
            }
        }

        // Если инструмент не найден (например, был удален), используем "Нет инструмента"
        if (!found) {
            var->add_instrument(m_noInstrument.get());
            item->setText("(нет инструмента)");
        }
    }

    // Обновляем таблицу измерений (чтобы показать/скрыть погрешности)
    m_tableModel->refreshData();
}

void MainWindow::createTestData()
{
    auto thermometer = std::make_shared<AbsoluteInstrument>("Термометр", 0.5);
    auto voltmeter = std::make_shared<RelativeInstrument>("Вольтметр", 0.02);

    // Добавляем инструменты после постоянного "Нет инструмента"
    m_instruments.push_back(thermometer);
    m_instruments.push_back(voltmeter);

    std::vector<double> temperatureData = {20.5, 21.2, 22.8, 23.1, 24.5, 25.0};
    std::vector<double> voltageData = {12.1, 12.3, 11.9, 12.5, 12.2, 12.4};
    std::vector<double> currentData = {1.5, 1.6, 1.55, 1.62, 1.58, 1.61};

    // По умолчанию используем "Нет инструмента"
    auto tempVar = std::make_shared<Variable>(temperatureData, "Температура", "T", m_noInstrument.get());
    auto voltageVar = std::make_shared<Variable>(voltageData, "Напряжение", "U", m_noInstrument.get());
    auto currentVar = std::make_shared<Variable>(currentData, "Ток", "I", m_noInstrument.get());

    m_experiment->add_variable(tempVar);
    m_experiment->add_variable(voltageVar);
    m_experiment->add_variable(currentVar);

    updateVariableInstrumentsTable();
}

void MainWindow::updateVariableInstrumentsTable()
{
    ui->variableInstrumentsTable->clear();
    ui->variableInstrumentsTable->setRowCount(1);
    ui->variableInstrumentsTable->setColumnCount(static_cast<int>(m_experiment->get_variables_count()));

    // Устанавливаем заголовки
    QStringList headers;
    for (size_t i = 0; i < m_experiment->get_variables_count(); ++i) {
        auto var = m_experiment->get_variable(i);
        QString headerName = QString::fromStdString(var->get_name_tables());
        QString tag = QString::fromStdString(var->get_name_calculated());

        if (!tag.isEmpty()) {
            headerName += "\n(" + tag + ")";
        }
        headers << headerName;
    }
    ui->variableInstrumentsTable->setHorizontalHeaderLabels(headers);

    // Заполняем данные об инструментах
    for (size_t i = 0; i < m_experiment->get_variables_count(); ++i) {
        auto var = m_experiment->get_variable(i);

        QTableWidgetItem* item = new QTableWidgetItem();
        try {
            QString instrumentName = QString::fromStdString(var->get_name_instrument());
            item->setText(instrumentName);
        } catch (const std::exception&) {
            item->setText("(нет инструмента)");
        }

        // Делаем ячейку редактируемой
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->variableInstrumentsTable->setItem(0, static_cast<int>(i), item);
    }
}

void MainWindow::addColumn()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Добавить переменную",
                                        "Введите название переменной:",
                                        QLineEdit::Normal, "Переменная", &ok);
    if (!ok || name.isEmpty()) return;

    // Создаем переменную с инструментом "Нет инструмента" по умолчанию
    std::vector<double> measurements(6, 0.0);
    auto newVar = std::make_shared<Variable>(measurements, name.toStdString(), "", m_noInstrument.get());
    m_experiment->add_variable(newVar);

    m_tableModel->refreshData();
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

    // Создаем инструмент с нулевой погрешностью (пользователь установит через таблицу)
    std::shared_ptr<Instrument> instrument = std::make_shared<AbsoluteInstrument>(name.toStdString(), 0.0);

    m_instruments.push_back(instrument);
    m_instrumentsModel->refreshData();

    // Обновляем делегат с новым инструментом
    updateInstrumentDelegate();
}

void MainWindow::removeInstrument()
{
    // Нельзя удалить инструмент "Нет инструмента"
    if (m_instruments.size() <= 1) {
        QMessageBox::information(this, "Информация", "Нельзя удалить базовый инструмент 'Нет инструмента'");
        return;
    }

    // Удаляем последний добавленный инструмент (кроме "Нет инструмента")
    auto instrumentToRemove = m_instruments.back();
    QString instrumentName = QString::fromStdString(instrumentToRemove->get_name());

    // Заменяем этот инструмент на "Нет инструмента" во всех переменных
    for (size_t i = 0; i < m_experiment->get_variables_count(); ++i) {
        auto var = m_experiment->get_variable(i);
        try {
            if (QString::fromStdString(var->get_name_instrument()) == instrumentName) {
                var->add_instrument(m_noInstrument.get());
                // Обновляем отображение в таблице
                QTableWidgetItem* item = ui->variableInstrumentsTable->item(0, static_cast<int>(i));
                if (item) {
                    item->setText("(нет инструмента)");
                }
            }
        } catch (const std::exception&) {
            // У переменной нет инструмента - ничего не делаем
        }
    }

    m_instruments.pop_back();
    m_instrumentsModel->refreshData();

    // Обновляем делегат
    updateInstrumentDelegate();

    // Обновляем таблицу измерений
    m_tableModel->refreshData();
}

MainWindow::~MainWindow()
{
    delete ui;
}
