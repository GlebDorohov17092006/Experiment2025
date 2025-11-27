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
#include "parser.h"
#include "qcustomplot.h"
#include "basesettingswidget.h"
#include "TextReportBlock.h"
#include "TableReportBlock.h"
#include "PlotReportBlock.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QSet>
#include <QMap>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QComboBox>
#include <QGridLayout>
#include <QPushButton>
#include <QFont>
#include <QTabBar>
#include <QFileDialog>
#include <QTextEdit>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QPrinter>
#include <QPrintDialog>

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

    // Подключаем сигналы из ветки Gleb
    connect(ui->addColumnButton, &QPushButton::clicked, this, &MainWindow::addColumn);
    connect(ui->removeColumnButton, &QPushButton::clicked, this, &MainWindow::removeColumn);
    connect(ui->addRowButton, &QPushButton::clicked, this, &MainWindow::addRow);
    connect(ui->removeRowButton, &QPushButton::clicked, this, &MainWindow::removeRow);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::addInstrument);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::removeInstrument);
    connect(ui->variableInstrumentsTable, &QTableWidget::itemChanged, this, &MainWindow::onInstrumentChanged);

    connect(m_instrumentsModel, &InstrumentsModel::instrumentNameChanged, this, &MainWindow::updateInstrumentDelegate);

    // Подключаем сигналы из ветки main (дополнительные функции)
    connect(ui->add_graph, &QAction::triggered, this, [this]() { addDynamicPlotTab("График"); });
    connect(ui->add_histogram, &QAction::triggered, this, [this]() { addDynamicPlotTab("Гистограмма"); });
    connect(ui->add_scatterplot, &QAction::triggered, this, [this]() { addDynamicPlotTab("Скаттерплот"); });
    connect(ui->delete_plot, &QAction::triggered, this, [this]() { removeGraph(); });

    // Подключаем обработчик для перетаскивания вкладок графиков
    connect(ui->tabPlot->tabBar(), &QTabBar::tabMoved, this, &MainWindow::onPlotTabMoved);

    // Настройка таблицы инструментов
    ui->tableViewInstruments->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableViewInstruments->setColumnWidth(0, 150);
    ui->tableViewInstruments->setColumnWidth(1, 300);
    ui->tableViewInstruments->setColumnWidth(2, 100);

    ui->variableInstrumentsTable->horizontalHeader()->setDefaultSectionSize(200);

    // Создаем графики по умолчанию
    addDynamicPlotTab("График");
    addDynamicPlotTab("Гистограмма");
    addDynamicPlotTab("Скаттерплот");

    // Подключаем обработчики двойного клика по заголовкам
    connect(ui->tableViewMeasurements->horizontalHeader(), &QHeaderView::sectionDoubleClicked, 
            this, &MainWindow::onColumnHeaderDoubleClicked);

    // Подключаем обработчики двойного клика по ячейкам инструментов
    connect(ui->variableInstrumentsTable, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::onInstrumentCellDoubleClicked);

    // Подключаем синхронизацию вкладок графиков и настроек
    connect(ui->tabPlot, &QTabWidget::currentChanged, this, &MainWindow::onPlotTabChanged);
    connect(ui->tabPlotSettings, &QTabWidget::currentChanged, this, &MainWindow::onPlotSettingsTabChanged);

    // Парсинг по умолчанию (из ветки main)
    std::vector<Variable> variables = parser("../../electricChain.csv", "../../errors_tools.json");
    Experiment::get_instance(variables, {});

    // Отображение графиков (из ветки main)
    if(m_plotTabs.size() >= 2)
    {
        PlotTab plot = m_plotTabs[0];

        QVector<double> xQvector = QVector<double>(variables[0].get_measurements().begin(), variables[0].get_measurements().end());
        QVector<double> yQvector = QVector<double>(variables[1].get_measurements().begin(), variables[1].get_measurements().end());

        plot.plot->addGraph();
        // Отключаем адаптивную выборку для правильного отображения ломаной линии
        plot.plot->graph()->setAdaptiveSampling(false);
        // Устанавливаем стиль линии как ломаную
        plot.plot->graph()->setLineStyle(QCPGraph::lsLine);
        plot.plot->graph()->setData(xQvector, yQvector);

        plot.plot->rescaleAxes();
        plot.plot->replot();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ========== МЕТОДЫ ИЗ ВЕТКИ GLEB (ОСНОВНАЯ ЛОГИКА) ==========

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
    experiment->add_variable(voltageData);
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
    syncPlotSettingsTables();
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
    syncPlotSettingsTables();
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

// ========== МЕТОДЫ ИЗ ВЕТКИ MAIN (ДОПОЛНИТЕЛЬНЫЙ ФУНКЦИОНАЛ) ==========

void MainWindow::onPlotSettingsTabChanged(int index)
{
    // Синхронизируем табы: когда меняется вкладка настроек, меняем вкладку графика
    ui->tabPlot->blockSignals(true);
    ui->tabPlot->setCurrentIndex(index);
    ui->tabPlot->blockSignals(false);
}

void MainWindow::onPlotTabChanged(int index)
{
    // Синхронизируем табы: когда меняется вкладка графика, меняем вкладку настроек
    ui->tabPlotSettings->blockSignals(true);
    ui->tabPlotSettings->setCurrentIndex(index);
    ui->tabPlotSettings->blockSignals(false);
}

void MainWindow::onPlotTabMoved(int from, int to)
{
    // Обновляем порядок элементов в m_plotTabs
    PlotTab movedTab = m_plotTabs[from];
    m_plotTabs.removeAt(from);
    m_plotTabs.insert(to, movedTab);

    // Синхронизируем перемещение вкладки настроек
    QTabBar* settingsTabBar = ui->tabPlotSettings->tabBar();
    settingsTabBar->moveTab(from, to);
}

void MainWindow::onColumnHeaderDoubleClicked(int col)
{
    QString oldName = getColumnName(col);
    QString oldTag = getColumnTag(col);

    QDialog dialog(this);
    dialog.setWindowTitle("Настройка столбца");

    QFormLayout *formLayout = new QFormLayout(&dialog);

    QLineEdit *nameEdit = new QLineEdit(oldName, &dialog);
    formLayout->addRow("Полное имя:", nameEdit);

    QLineEdit *tagEdit = new QLineEdit(oldTag, &dialog);
    QLabel *tagHint = new QLabel("", &dialog);
    tagHint->setStyleSheet("color: gray; font-size: 9pt;");
    formLayout->addRow("Тег:", tagEdit);
    formLayout->addRow("", tagHint);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formLayout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString newName = nameEdit->text().trimmed();
        QString newTag = tagEdit->text().trimmed();

        if (!newName.isEmpty()) {
            setColumnTag(col, newTag);

            QString headerText = newName;
            if (!newTag.isEmpty()) {
                headerText += "\n(" + newTag + ")";
            }

            // Обновляем заголовки во всех таблицах
            updateVariableInstrumentsTable();
            
            // Обновляем вертикальный заголовок в таблицах настроек динамически добавленных графиков
            for (auto& plotTab : m_plotTabs) {
                if (col < plotTab.settingsTable->rowCount()) {
                    plotTab.settingsTable->setVerticalHeaderItem(col, new QTableWidgetItem(newName));
                }
            }
        }
    }
}

QString MainWindow::getColumnName(int columnIndex)
{
    auto experiment = Experiment::get_instance();
    if (columnIndex < static_cast<int>(experiment->get_variables_count())) {
        auto& var = experiment->get_variable(columnIndex);
        return QString::fromStdString(var.get_name_tables());
    } else {
        return QString("Столбец %1").arg(columnIndex + 1);
    }
}

void MainWindow::setColumnTag(int columnIndex, const QString& tag)
{
    if (tag.isEmpty()) {
        m_columnTags.remove(columnIndex);
    } else {
        m_columnTags[columnIndex] = tag;
    }
}

QString MainWindow::getColumnTag(int columnIndex)
{
    return m_columnTags.value(columnIndex, "");
}

void MainWindow::syncPlotSettingsTables()
{
    auto experiment = Experiment::get_instance();
    int mainTableColumns = static_cast<int>(experiment->get_variables_count());

    // Синхронизируем таблицы настроек динамически добавленных графиков
    for (auto& plotTab : m_plotTabs) {
        // Добавляем недостающие строки
        while (plotTab.settingsTable->rowCount() < mainTableColumns) {
            QString columnName = getColumnName(plotTab.settingsTable->rowCount());
            int rowIndex = plotTab.settingsTable->rowCount();
            plotTab.settingsTable->insertRow(rowIndex);
            plotTab.settingsTable->setVerticalHeaderItem(rowIndex, new QTableWidgetItem(columnName));

            QTableWidgetItem* checkItem = new QTableWidgetItem();
            checkItem->setCheckState(Qt::Checked);
            plotTab.settingsTable->setItem(rowIndex, BaseSettingsWidget::ColumnEnabled, checkItem);

            // Добавляем остальные колонки
            int columnCount = plotTab.settingsTable->columnCount();
            for (int j = 1; j < columnCount; ++j) {
                plotTab.settingsTable->setItem(rowIndex, j, new QTableWidgetItem(""));
            }
        }

        // Обновляем вертикальные заголовки
        for (int i = 0; i < mainTableColumns; ++i) {
            QString columnName = getColumnName(i);
            if (i < plotTab.settingsTable->rowCount()) {
                plotTab.settingsTable->setVerticalHeaderItem(i, new QTableWidgetItem(columnName));
            }
        }
    }
}

void MainWindow::onInstrumentCellDoubleClicked(int row, int column)
{
    if (row != 0) return; // У нас только одна строка

    QTableWidgetItem* item = ui->variableInstrumentsTable->item(row, column);
    if (!item) return;

    // Используем делегат для редактирования
    ui->variableInstrumentsTable->editItem(item);
}

void MainWindow::addDynamicPlotTab(const QString& plotType)
{
    QString baseName = plotType;
    QString tabName;

    // Проверяем, есть ли уже график с базовым именем (без номера)
    bool hasBaseName = false;
    for (const auto& tab : m_plotTabs) {
        if (tab.name == baseName) {
            hasBaseName = true;
            break;
        }
    }

    // Если базового имени нет, используем его, иначе ищем минимальный свободный номер
    if (!hasBaseName) {
        tabName = baseName;
    } else {
        int counter = 1;
        bool nameExists;
        do {
            tabName = baseName + " " + QString::number(counter);
            nameExists = false;

            for (const auto& tab : m_plotTabs) {
                if (tab.name == tabName) {
                    nameExists = true;
                    counter++;
                    break;
                }
            }
        } while (nameExists);
    }

    // Создаем новый виджет для вкладки графика
    QWidget* plotWidget = new QWidget();
    QGridLayout* plotLayout = new QGridLayout(plotWidget);
    QCustomPlot* plot = new QCustomPlot(plotWidget);
    plot->setInteraction(QCP::iRangeDrag, true);
    plot->setInteraction(QCP::iRangeZoom, true);
    plotLayout->addWidget(plot, 0, 0);

    // Создаем вкладку в tabPlot
    ui->tabPlot->addTab(plotWidget, tabName);

    // Создаем соответствующий виджет настроек
    BaseSettingsWidget* settingsWidget = BaseSettingsWidget::create(plotType);
    if (!settingsWidget) {
        return;
    }

    QTableWidget* settingsTable = settingsWidget->settingsTable();
    if (!settingsTable) {
        delete settingsWidget;
        return;
    }

    // Настраиваем делегаты для редактирования ячеек
    settingsWidget->setupDelegates(this);

    auto experiment = Experiment::get_instance();
    int columnCount = static_cast<int>(experiment->get_variables_count());
    int tableColumnCount = settingsTable->columnCount();

    for (int i = 0; i < columnCount; ++i) {
        QString columnName = getColumnName(i);
        int rowIndex = settingsTable->rowCount();
        settingsTable->insertRow(rowIndex);
        settingsTable->setVerticalHeaderItem(rowIndex, new QTableWidgetItem(columnName));

        QTableWidgetItem* checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Checked);
        settingsTable->setItem(rowIndex, BaseSettingsWidget::ColumnEnabled, checkItem);

        for (int j = 1; j < tableColumnCount; ++j) {
            settingsTable->setItem(rowIndex, j, new QTableWidgetItem(""));
        }
    }

    ui->tabPlotSettings->addTab(settingsWidget, tabName);

    // Сохраняем информацию о графике
    PlotTab plotTab;
    plotTab.name = tabName;
    plotTab.type = plotType;
    plotTab.plot = plot;
    plotTab.settingsTab = settingsWidget;
    plotTab.settingsTable = settingsTable;
    m_plotTabs.append(plotTab);
}

void MainWindow::removeGraph(int index)
{
    int currentIndex = (index == -1) ? ui->tabPlot->currentIndex() : index;

    if (currentIndex < 0 || currentIndex >= m_plotTabs.size()) {
        return;
    }

    QString graphName = m_plotTabs[currentIndex].name;
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Подтверждение удаления");
    msgBox.setText(QString("Вы действительно хотите удалить график '%1'?").arg(graphName));
    QPushButton* yesButton = msgBox.addButton("Да", QMessageBox::AcceptRole);
    QPushButton* noButton = msgBox.addButton("Нет", QMessageBox::RejectRole);
    msgBox.setDefaultButton(yesButton);
    msgBox.exec();

    if (msgBox.clickedButton() == yesButton) {
        ui->tabPlot->removeTab(currentIndex);

        int settingsTabIndex = -1;
        for (int i = 0; i < ui->tabPlotSettings->count(); ++i) {
            if (ui->tabPlotSettings->widget(i) == m_plotTabs[currentIndex].settingsTab) {
                settingsTabIndex = i;
                break;
            }
        }

        if (settingsTabIndex >= 0) {
            ui->tabPlotSettings->removeTab(settingsTabIndex);
        }

        m_plotTabs.removeAt(currentIndex);
    }
}

void MainWindow::on_import_CSV_triggered()
{
    //Creating dialog window
    QFileDialog dialog(this);

    //Setting design of dialog window
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter("*.json *.csv");

    //Extracting paths of files
    if(dialog.exec() == QFileDialog::Accepted)
    {
        QString csvFile, jsonFile;

        QStringList files = dialog.selectedFiles();
        if(files.size() == 2)
        {
            for(const QString &filePath : files)
            {
                if(filePath.endsWith(".csv", Qt::CaseInsensitive))
                {
                    csvFile = filePath;
                }
                else
                {
                    jsonFile = filePath;
                }
            }

            //Parsing files
            auto variables = parser(csvFile.toStdString(), jsonFile.toStdString());
            Experiment::destroy_instance();
            Experiment* experiment = Experiment::get_instance(variables, std::vector<Variable>());
            
            // Обновляем интерфейс после импорта
            m_tableModel->refreshData();
            updateVariableInstrumentsTable();
            syncPlotSettingsTables();
        }
        else
        {
            QMessageBox::critical(this,
                "Ошибка выбора файлов",
                "Пожалуйста, выберите ровно два файла: один CSV и один JSON");

            return;
        }
    }
}

void MainWindow::addTextBlockToReport()
{
    QVBoxLayout* layout = ui->reportContentLayout;
    if (!layout) {
        return;
    }
    
    QWidget* contentWidget = ui->reportContentWidget;

    TextReportBlock* block = new TextReportBlock(contentWidget);
    QFrame* frame = block->createWidget(contentWidget, layout);
    m_reportBlocks.append(block);
    
    layout->addWidget(frame);
    
    connectReportBlockDeletion(frame, block);
}

void MainWindow::addTableBlockToReport()
{
    QVBoxLayout* layout = ui->reportContentLayout;
    if (!layout) {
        return;
    }
    
    QWidget* contentWidget = ui->reportContentWidget;

    TableReportBlock* block = new TableReportBlock(m_tableModel, contentWidget);
    QFrame* frame = block->createWidget(contentWidget, layout);
    m_reportBlocks.append(block);
    
    layout->addWidget(frame);
    
    connectReportBlockDeletion(frame, block);
}

void MainWindow::addPlotBlockToReport()
{
    QVBoxLayout* layout = ui->reportContentLayout;
    if (!layout) {
        return;
    }
    
    QWidget* contentWidget = ui->reportContentWidget;

    int currentIndex = ui->tabPlot->currentIndex();
    if (currentIndex < 0 || currentIndex >= m_plotTabs.size()) {
        QMessageBox::information(this, "Информация", "Нет графиков для добавления в отчет");
        return;
    }

    QCustomPlot* sourcePlot = m_plotTabs[currentIndex].plot;

    int fixedSize = 400;
    QPixmap pixmap = sourcePlot->toPixmap(fixedSize, fixedSize);

    PlotReportBlock* block = new PlotReportBlock(pixmap, contentWidget);
    QFrame* frame = block->createWidget(contentWidget, layout);
    m_reportBlocks.append(block);
    
    layout->addWidget(frame);
    
    connectReportBlockDeletion(frame, block);
}

void MainWindow::connectReportBlockDeletion(QFrame* frame, ReportBlock* block)
{
    // Сигнал удаления для очистки списка и освобождения памяти
    connect(frame, &QFrame::destroyed, this, [this, block]() {
        m_reportBlocks.removeAll(block);
        delete block;
    });
}

void MainWindow::saveReport()
{
    if (m_reportBlocks.isEmpty()) {
        QMessageBox::information(this, "Информация", "Нет блоков для сохранения в отчете");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Сохранить отчет",
        "",
        "PDF Files (*.pdf);;All Files (*)"
        );

    if (fileName.isEmpty()) {
        return;
    }

    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
        fileName += ".pdf";
    }

    QTextDocument document;
    QTextCursor cursor(&document);

    QVBoxLayout* layout = ui->reportContentLayout;

    QHash<QFrame*, ReportBlock*> frameToBlock;
    for (ReportBlock* block : m_reportBlocks) {
        if (block && block->getFrame()) {
            frameToBlock[block->getFrame()] = block;
        }
    }

    for (int i = 0; i < layout->count(); ++i) {
        QLayoutItem* item = layout->itemAt(i);

        QWidget* widget = item->widget();

        QFrame* frame = qobject_cast<QFrame*>(widget);

        ReportBlock* block = frameToBlock.value(frame, nullptr);

        block->exportToPDF(&document, cursor);

        if (i < layout->count() - 1) {
            cursor.insertBlock();
        }
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize::A4);
    printer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);

    document.print(&printer);

    QMessageBox::information(this, "", QString("Отчет успешно сохранен в файл:\n%1").arg(fileName));
}