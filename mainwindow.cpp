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
#include "json.hpp"
#include "qcustomplot.h"
#include "basesettingswidget.h"
#include "TextReportBlock.h"
#include "TableReportBlock.h"
#include "PlotReportBlock.h"
#include "plotsettingswidget.h"

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
#include <map>
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
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QDebug>

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

    // Создаем пустой эксперимент при запуске
    Experiment::destroy_instance();
    Experiment::get_instance(std::vector<Variable>(), std::vector<Variable>());

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
    // addColumnButton, removeColumnButton, addRowButton, removeRowButton подключены через UI файл, не дублируем здесь
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
}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::updateVariableInstrumentsTable()
{
    auto experiment = Experiment::get_instance();
    if (!experiment) {
        return;
    }

    size_t variablesCount = experiment->get_variables_count();

    ui->variableInstrumentsTable->clear();
    ui->variableInstrumentsTable->setRowCount(1);
    ui->variableInstrumentsTable->setColumnCount(static_cast<int>(variablesCount));

    QStringList headers;
    for (size_t i = 0; i < variablesCount; ++i) {
        auto& var = experiment->get_variable(i);
        QString headerName = QString::fromStdString(var.get_name_tables());
        QString tag = QString::fromStdString(var.get_name_calculated());

        if (!tag.isEmpty()) {
            headerName += "\n(" + tag + ")";
        }
        headers << headerName;
        
        // Если у переменной нет инструмента (nullptr), устанавливаем инструмент по умолчанию
        if (var.get_instrument() == nullptr) {
            var.add_instrument(m_noInstrument.get());
        }
    }
    ui->variableInstrumentsTable->setHorizontalHeaderLabels(headers);

    // Блокируем сигналы, чтобы избежать вызова onInstrumentChanged при установке текста
    ui->variableInstrumentsTable->blockSignals(true);
    
    for (size_t i = 0; i < variablesCount; ++i) {
        auto& var = experiment->get_variable(i);

        QTableWidgetItem* item = new QTableWidgetItem();
        QString instrumentName = QString::fromStdString(var.get_name_instrument());
        item->setText(instrumentName);
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        ui->variableInstrumentsTable->setItem(0, static_cast<int>(i), item);
    }
    
    // Разблокируем сигналы
    ui->variableInstrumentsTable->blockSignals(false);

    // Растягиваем заголовки
    ui->variableInstrumentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::addColumn()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Добавить переменную", "Введите название переменной:", QLineEdit::Normal, "Переменная", &ok);
    if (!ok || name.isEmpty()) return;

    auto experiment = Experiment::get_instance();

    std::vector<double> measurements; // Пустой вектор - без измерений
    Variable newVar(measurements, name.toStdString(), "", m_noInstrument.get());
    experiment->add_variable(newVar);

    m_tableModel->refreshData();
    updateVariableInstrumentsTable();
    syncPlotSettingsTables();
    
    // Добавляем новую переменную в combobox для оси Y во всех графиках
    for (PlotTab& plotTab : m_plotTabs) {
        if (plotTab.type == "График") {
            PlotSettingsWidget* plotSettings = qobject_cast<PlotSettingsWidget*>(plotTab.settingsTab);
            if (plotSettings) {
                QComboBox* yAxisCombo = plotSettings->yAxisComboBox();
                if (yAxisCombo) {
                    QString varName = name;
                    varName.remove('\"');
                    if (varName.isEmpty()) {
                        varName = QString("Переменная %1").arg(experiment->get_variables_count());
                    }
                    int varIndex = static_cast<int>(experiment->get_variables_count() - 1);
                    yAxisCombo->addItem(varName, varIndex);
                }
            }
        }
    }
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
    if (!experiment) {
        return QString("Столбец %1").arg(columnIndex + 1);
    }

    if (columnIndex >= 0 && columnIndex < static_cast<int>(experiment->get_variables_count())) {
        auto& var = experiment->get_variable(columnIndex);
        QString name = QString::fromStdString(var.get_name_tables());
        if (name.isEmpty()) {
            name = QString("Переменная %1").arg(columnIndex + 1);
        }
        return name;
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
    if (!experiment) {
        return;
    }

    int mainTableColumns = static_cast<int>(experiment->get_variables_count());

    // Синхронизируем таблицы настроек динамически добавленных графиков
    for (auto& plotTab : m_plotTabs) {
        if (!plotTab.settingsTable) {
            continue;
        }

        int currentRows = plotTab.settingsTable->rowCount();

        // Добавляем недостающие строки
        if (currentRows < mainTableColumns) {
            for (int i = currentRows; i < mainTableColumns; ++i) {
                QString columnName = getColumnName(i);
                plotTab.settingsTable->insertRow(i);
                plotTab.settingsTable->setVerticalHeaderItem(i, new QTableWidgetItem(columnName));

                // Добавляем значения по умолчанию
                if (plotTab.type == "График") {
                    QTableWidgetItem* checkItem = new QTableWidgetItem();
                    checkItem->setCheckState(Qt::Unchecked); // Графики не отображаются по умолчанию
                    plotTab.settingsTable->setItem(i, 0, checkItem);

                    // Тип линии по умолчанию
                    plotTab.settingsTable->setItem(i, 1, new QTableWidgetItem("Сплошная"));

                    // Ширина линии по умолчанию
                    plotTab.settingsTable->setItem(i, 2, new QTableWidgetItem("1"));

                    // Тип точки по умолчанию
                    plotTab.settingsTable->setItem(i, 3, new QTableWidgetItem("Круг"));

                    // Размер точки по умолчанию
                    plotTab.settingsTable->setItem(i, 4, new QTableWidgetItem("6"));

                    // Цвет по умолчанию (синий)
                    QColor defaultColor = QColor::fromHsv((i * 60) % 360, 255, 255);
                    plotTab.settingsTable->setItem(i, 5, new QTableWidgetItem(defaultColor.name()));
                }
            }
        }
        // Удаляем лишние строки
        else if (currentRows > mainTableColumns) {
            for (int i = currentRows - 1; i >= mainTableColumns; --i) {
                plotTab.settingsTable->removeRow(i);
            }
        }

        // Обновляем вертикальные заголовки
        for (int i = 0; i < mainTableColumns; ++i) {
            QString columnName = getColumnName(i);
            plotTab.settingsTable->setVerticalHeaderItem(i, new QTableWidgetItem(columnName));
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

    ui->tabPlotSettings->addTab(settingsWidget, tabName);

    // Сохраняем информацию о графике
    PlotTab plotTab;
    plotTab.name = tabName;
    plotTab.type = plotType;
    plotTab.plot = plot;
    plotTab.settingsTab = settingsWidget;
    plotTab.settingsTable = settingsTable;
    m_plotTabs.append(plotTab);

    // Настраиваем специфичные настройки для графиков
    PlotSettingsWidget* plotSettings = qobject_cast<PlotSettingsWidget*>(settingsWidget);
    if (plotSettings) {
            // Синхронизируем таблицу с переменными (создает строки для всех переменных)
            // Вызываем после добавления plotTab в m_plotTabs, чтобы syncPlotSettingsTables мог найти новый график
            syncPlotSettingsTables();

            // Заполняем ComboBox переменными из Experiment
            updateVariableComboBoxes(plotSettings);

            // Устанавливаем настройки по умолчанию (без галочек - графики не отображаются по умолчанию)
            Experiment* experiment = Experiment::get_instance();
            if (experiment && experiment->get_variables_count() > 0) {
                // Убеждаемся, что все галочки сняты (графики не отображаются по умолчанию)
                QTableWidget* settingsTable = plotSettings->settingsTable();
                if (settingsTable && settingsTable->rowCount() > 0) {
                    for (int i = 0; i < settingsTable->rowCount(); ++i) {
                        QTableWidgetItem* item = settingsTable->item(i, PlotSettingsWidget::ColumnEnabled);
                        if (item) {
                            item->setCheckState(Qt::Unchecked);
                        }
                    }
                }

                // Устанавливаем названия осей по умолчанию
                if (experiment->get_variables_count() >= 2) {
                    QString xVarName = QString::fromStdString(experiment->get_variable(0).get_name_tables());
                    xVarName.remove('\"');
                    QString yVarName = QString::fromStdString(experiment->get_variable(1).get_name_tables());
                    yVarName.remove('\"');
                    plotSettings->xAxisLabelEdit()->setText(xVarName);
                    plotSettings->yAxisLabelEdit()->setText(yVarName);
                } else if (experiment->get_variables_count() == 1) {
                    QString xVarName = QString::fromStdString(experiment->get_variable(0).get_name_tables());
                    xVarName.remove('\"');
                    plotSettings->xAxisLabelEdit()->setText(xVarName);
                    plotSettings->yAxisLabelEdit()->setText(xVarName);
                }
            }
    }

    // Подключаем обработчик изменений в таблице настроек для графиков
    if (plotType == "График") {
        PlotSettingsWidget* plotSettings = qobject_cast<PlotSettingsWidget*>(settingsWidget);
        if (plotSettings) {
            // Используем индекс последнего добавленного элемента
            int tabIndex = m_plotTabs.size() - 1;

            // Обработчик изменений в таблице настроек
            connect(settingsTable, &QTableWidget::cellChanged, this, [this, tabIndex](int row, int column) {
                if (tabIndex >= 0 && tabIndex < m_plotTabs.size()) {
                    PlotTab& plotTab = m_plotTabs[tabIndex];
                    PlotSettingsWidget* plotSettings = qobject_cast<PlotSettingsWidget*>(plotTab.settingsTab);
                    if (!plotSettings) {
                        return;
                    }

                    // Если изменилась галочка (ColumnEnabled), перестраиваем все графики
                    if (column == PlotSettingsWidget::ColumnEnabled) {
                        // Разрешаем несколько галочек - каждая создает свой график
                        rebuildPlotFromSettings(tabIndex);
                    } else if (row >= 0) {
                        // Для других изменений применяем настройки стиля к соответствующему графику
                        QTableWidget* table = plotSettings->settingsTable();
                        QTableWidgetItem* enabledItem = table->item(row, PlotSettingsWidget::ColumnEnabled);
                        if (enabledItem && enabledItem->checkState() == Qt::Checked) {
                            // Находим график, соответствующий этой строке
                            QCPGraph* graph = findGraphForVariable(plotTab.plot, row);
                            if (graph) {
                                applyPlotSettingsFromTable(graph, table, row);
                                plotTab.plot->replot();
                            }
                        }
                    }
                }
            });

            // Обработчик изменения выбора переменной Y в ComboBox
            QComboBox* yAxisCombo = plotSettings->yAxisComboBox();
            connect(yAxisCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, tabIndex, plotSettings]() {
                if (tabIndex >= 0 && tabIndex < m_plotTabs.size()) {
                    rebuildPlotFromSettings(tabIndex);
                }
            });

            // Обработчик изменения названий осей
            connect(plotSettings->xAxisLabelEdit(), &QLineEdit::textChanged, this, [this, tabIndex]() {
                if (tabIndex >= 0 && tabIndex < m_plotTabs.size()) {
                    PlotTab& plotTab = m_plotTabs[tabIndex];
                    PlotSettingsWidget* plotSettings = qobject_cast<PlotSettingsWidget*>(plotTab.settingsTab);
                    if (plotSettings && plotTab.plot) {
                        plotTab.plot->xAxis->setLabel(plotSettings->xAxisLabelEdit()->text());
                        plotTab.plot->replot();
                    }
                }
            });

            connect(plotSettings->yAxisLabelEdit(), &QLineEdit::textChanged, this, [this, tabIndex]() {
                if (tabIndex >= 0 && tabIndex < m_plotTabs.size()) {
                    PlotTab& plotTab = m_plotTabs[tabIndex];
                    PlotSettingsWidget* plotSettings = qobject_cast<PlotSettingsWidget*>(plotTab.settingsTab);
                    if (plotSettings && plotTab.plot) {
                        plotTab.plot->yAxis->setLabel(plotSettings->yAxisLabelEdit()->text());
                        plotTab.plot->replot();
                    }
                }
            });
        }
    }
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

void MainWindow::on_import_data_triggered()
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
                else if(filePath.endsWith(".json", Qt::CaseInsensitive))
                {
                    jsonFile = filePath;
                }
            }

            // Проверяем, что оба файла выбраны
            if(csvFile.isEmpty() || jsonFile.isEmpty()) {
                QMessageBox::critical(this, "Ошибка",
                    "Необходимо выбрать один CSV и один JSON файл");
                return;
            }

            //Parsing files
            QApplication::setOverrideCursor(Qt::WaitCursor);
            QApplication::processEvents();

            std::vector<Variable> variables;
            try {
                variables = parser(csvFile.toStdString(), jsonFile.toStdString());
            } catch (const std::exception& e) {
                QApplication::restoreOverrideCursor();
                QMessageBox::critical(this, "Ошибка парсинга",
                    QString("Произошла ошибка при парсинге файлов:\n%1").arg(e.what()));
                return;
            }

            // Проверяем, что данные были получены
            if (variables.empty()) {
                QApplication::restoreOverrideCursor();
                QMessageBox::warning(this, "Предупреждение",
                    "Не удалось загрузить данные из файлов.\nВозможно, файлы пусты или имеют неправильный формат.");
                return;
            }

            // Уничтожаем старый эксперимент
            Experiment::destroy_instance();

            // Создаем новый эксперимент с данными из парсера
            Experiment* experiment = Experiment::get_instance(variables, std::vector<Variable>());

            // Обрабатываем события UI
            QApplication::processEvents();

            // Обновляем модель таблицы
            m_tableModel->refreshData();

            // Обновляем таблицу инструментов переменных
            updateVariableInstrumentsTable();

            // Обновляем ComboBox во всех графиках
            for (PlotTab& plotTab : m_plotTabs) {
                if (plotTab.type == "График") {
                    PlotSettingsWidget* plotSettings = qobject_cast<PlotSettingsWidget*>(plotTab.settingsTab);
                    if (plotSettings) {
                        updateVariableComboBoxes(plotSettings);

                        // Устанавливаем названия осей по умолчанию (без автоматического построения графика)
                        if (experiment->get_variables_count() >= 2) {
                            int xIndex = 0;
                            int yIndex = 1;

                            // Убеждаемся, что все галочки сняты (графики не отображаются по умолчанию)
                            QTableWidget* settingsTable = plotSettings->settingsTable();
                            if (settingsTable && settingsTable->rowCount() > xIndex) {
                                for (int i = 0; i < settingsTable->rowCount(); ++i) {
                                    QTableWidgetItem* item = settingsTable->item(i, PlotSettingsWidget::ColumnEnabled);
                                    if (item) {
                                        item->setCheckState(Qt::Unchecked);
                                    }
                                }
                            }

                            // Устанавливаем переменную Y в ComboBox
                            QComboBox* yAxisCombo = plotSettings->yAxisComboBox();
                            if (yAxisCombo && yAxisCombo->count() > yIndex) {
                                yAxisCombo->setCurrentIndex(yIndex);
                            }

                            // Устанавливаем названия осей по умолчанию
                            QString xVarName = QString::fromStdString(experiment->get_variable(xIndex).get_name_tables());
                            xVarName.remove('\"');
                            QString yVarName = QString::fromStdString(experiment->get_variable(yIndex).get_name_tables());
                            yVarName.remove('\"');
                            plotSettings->xAxisLabelEdit()->setText(xVarName);
                            plotSettings->yAxisLabelEdit()->setText(yVarName);

                            // Очищаем график, так как галочки не установлены
                            if (plotTab.plot) {
                                plotTab.plot->clearGraphs();
                                plotTab.plot->replot();
                            }
                        }
                    }
                }
            }

            // Синхронизируем таблицы настроек графиков
            syncPlotSettingsTables();

            QApplication::restoreOverrideCursor();
            QMessageBox::information(this, "Успешно",
                QString("Файлы успешно загружены и обработаны.\n"
                       "Загружено: %1 переменных\n"
                       "Измерений: %2")
                .arg(variables.size())
                .arg(variables.empty() ? 0 : variables[0].get_measurements_count()));
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

void MainWindow::updateVariableComboBoxes(PlotSettingsWidget* plotSettings)
{
    if (!plotSettings) {
        return;
    }

    Experiment* experiment = Experiment::get_instance();
    if (!experiment) {
        return;
    }

    QComboBox* yAxisCombo = plotSettings->yAxisComboBox();

    if (!yAxisCombo) {
        return;
    }

    // Сохраняем текущий выбранный индекс (если ComboBox уже заполнен)
    int currentYIndex = -1;
    if (yAxisCombo->count() > 0) {
        currentYIndex = yAxisCombo->currentData().toInt();
    }

    yAxisCombo->clear();

    if (experiment->get_variables_count() == 0) {
        return;
    }

    for (size_t i = 0; i < experiment->get_variables_count(); ++i) {
        auto variable = experiment->get_variable(i);
        QString varName = QString::fromStdString(variable.get_name_tables());
        // Убираем лишние кавычки из названий переменных для отображения в ComboBox
        varName.remove('\"');
        if (varName.isEmpty()) {
            varName = QString("Переменная %1").arg(i + 1);
        }
        yAxisCombo->addItem(varName, static_cast<int>(i));
    }

    // Устанавливаем значение по умолчанию, если оно не было сохранено
    if (currentYIndex < 0 || currentYIndex >= static_cast<int>(experiment->get_variables_count())) {
        currentYIndex = (experiment->get_variables_count() > 1) ? 1 : 0; // Вторая переменная по умолчанию, или первая если только одна
    }

    // Устанавливаем выбранный индекс
    int yComboIndex = yAxisCombo->findData(currentYIndex);
    if (yComboIndex >= 0) {
        yAxisCombo->setCurrentIndex(yComboIndex);
    } else if (yAxisCombo->count() > 0) {
        yAxisCombo->setCurrentIndex((yAxisCombo->count() > 1) ? 1 : 0);
    }
}

void MainWindow::applyPlotSettingsFromTable(QCPGraph* graph, QTableWidget* settingsTable, int rowIndex)
{
    if (!graph || !settingsTable || rowIndex < 0 || rowIndex >= settingsTable->rowCount()) {
        return;
    }

    // Проверяем, включена ли отрисовка
    QTableWidgetItem* enabledItem = settingsTable->item(rowIndex, PlotSettingsWidget::ColumnEnabled);
    if (enabledItem && enabledItem->checkState() != Qt::Checked) {
        graph->setVisible(false);
        return;
    }
    graph->setVisible(true);

    // Тип линии
    QTableWidgetItem* lineTypeItem = settingsTable->item(rowIndex, PlotSettingsWidget::ColumnLineType);
    if (lineTypeItem) {
        QString lineType = lineTypeItem->text();
        if (lineType == "Сплошная" || lineType == "line") {
            graph->setLineStyle(QCPGraph::lsLine);
        } else if (lineType == "Пунктирная" || lineType == "none") {
            graph->setLineStyle(QCPGraph::lsNone);
        } else if (lineType == "Ступенчатая" || lineType == "step") {
            graph->setLineStyle(QCPGraph::lsStepLeft);
        }
    }

    // Ширина линии
    QTableWidgetItem* widthItem = settingsTable->item(rowIndex, PlotSettingsWidget::ColumnWidth);
    if (widthItem) {
        bool ok;
        double width = widthItem->text().toDouble(&ok);
        if (ok && width > 0) {
            QPen pen = graph->pen();
            pen.setWidthF(width);
            graph->setPen(pen);
        }
    }

    // Тип точки
    QTableWidgetItem* pointTypeItem = settingsTable->item(rowIndex, PlotSettingsWidget::ColumnPointType);
    QCPScatterStyle scatterStyle = graph->scatterStyle();
    if (pointTypeItem) {
        QString pointType = pointTypeItem->text();
        QCPScatterStyle::ScatterShape shape = QCPScatterStyle::ssNone;

        if (pointType == "Круг" || pointType == "circle") {
            shape = QCPScatterStyle::ssCircle;
        } else if (pointType == "Квадрат" || pointType == "square") {
            shape = QCPScatterStyle::ssSquare;
        } else if (pointType == "Крестик" || pointType == "cross") {
            shape = QCPScatterStyle::ssCross;
        } else if (pointType == "Плюс" || pointType == "plus") {
            shape = QCPScatterStyle::ssPlus;
        } else if (pointType == "Ромб" || pointType == "diamond") {
            shape = QCPScatterStyle::ssDiamond;
        } else if (pointType == "Без точки" || pointType == "none") {
            shape = QCPScatterStyle::ssNone;
        }

        scatterStyle.setShape(shape);
    }

    // Размер точки
    QTableWidgetItem* pointSizeItem = settingsTable->item(rowIndex, PlotSettingsWidget::ColumnPointSize);
    if (pointSizeItem) {
        bool ok;
        double size = pointSizeItem->text().toDouble(&ok);
        if (ok && size > 0) {
            scatterStyle.setSize(size);
        }
    }

    // Цвет
    QTableWidgetItem* colorItem = settingsTable->item(rowIndex, PlotSettingsWidget::ColumnColor);
    if (colorItem) {
        QString colorString = colorItem->text();
        QColor color(colorString);
        if (color.isValid()) {
            QPen pen = graph->pen();
            pen.setColor(color);
            graph->setPen(pen);

            scatterStyle.setPen(QPen(color));
            scatterStyle.setBrush(QBrush(color));
        }
    }

    graph->setScatterStyle(scatterStyle);
}

QCPGraph* MainWindow::findGraphForVariable(QCustomPlot* plot, int variableIndex)
{
    if (!plot) {
        return nullptr;
    }
    
    // Ищем график с именем, соответствующим индексу переменной
    for (int i = 0; i < plot->graphCount(); ++i) {
        QCPGraph* graph = plot->graph(i);
        // Используем имя графика для хранения индекса переменной X
        QString graphName = graph->name();
        bool ok;
        int graphVarIndex = graphName.toInt(&ok);
        if (ok && graphVarIndex == variableIndex) {
            return graph;
        }
    }
    
    return nullptr;
}

void MainWindow::rebuildPlotFromSettings(int plot_tab_index)
{
    if (plot_tab_index < 0 || plot_tab_index >= m_plotTabs.size()) {
        return;
    }

    PlotTab& plotTab = m_plotTabs[plot_tab_index];
    PlotSettingsWidget* plotSettings = qobject_cast<PlotSettingsWidget*>(plotTab.settingsTab);
    if (!plotSettings) {
        return;
    }

    Experiment* experiment = Experiment::get_instance();
    if (!experiment || experiment->get_variables_count() == 0) {
        return;
    }

    // Получаем переменную для оси Y из ComboBox
    QComboBox* yAxisCombo = plotSettings->yAxisComboBox();
    if (!yAxisCombo || yAxisCombo->count() == 0) {
        return;
    }
    int yIndex = yAxisCombo->currentData().toInt();

    if (yIndex < 0 || yIndex >= static_cast<int>(experiment->get_variables_count())) {
        return;
    }

    QTableWidget* settingsTable = plotSettings->settingsTable();
    
    // Собираем список переменных X с установленными галочками
    QList<int> enabledXIndices;
    for (int i = 0; i < settingsTable->rowCount(); ++i) {
        QTableWidgetItem* enabledItem = settingsTable->item(i, PlotSettingsWidget::ColumnEnabled);
        if (enabledItem && enabledItem->checkState() == Qt::Checked) {
            if (i < static_cast<int>(experiment->get_variables_count())) {
                enabledXIndices.append(i);
            }
        }
    }

    // Удаляем графики, для которых галочка снята
    QList<QCPGraph*> graphsToRemove;
    for (int i = 0; i < plotTab.plot->graphCount(); ++i) {
        QCPGraph* graph = plotTab.plot->graph(i);
        QString graphName = graph->name();
        bool ok;
        int graphVarIndex = graphName.toInt(&ok);
        if (ok && !enabledXIndices.contains(graphVarIndex)) {
            graphsToRemove.append(graph);
        }
    }
    
    for (QCPGraph* graph : graphsToRemove) {
        plotTab.plot->removeGraph(graph);
    }

    // Создаем или обновляем графики для всех установленных галочек
    for (int xIndex : enabledXIndices) {
        QCPGraph* graph = findGraphForVariable(plotTab.plot, xIndex);
        
        if (!graph) {
            // Создаем новый график
            graph = plotTab.plot->addGraph();
            graph->setName(QString::number(xIndex)); // Сохраняем индекс переменной X в имени графика
        }
        
        // Строим или обновляем график
        draw_single_graph(xIndex, yIndex, plot_tab_index, graph);
    }

    // Обновляем оси и перерисовываем
    if (plotTab.plot->graphCount() > 0) {
        // Настраиваем названия осей из QLineEdit
        PlotSettingsWidget* plotSettingsWidget = qobject_cast<PlotSettingsWidget*>(plotTab.settingsTab);
        if (plotSettingsWidget) {
            QString xLabel = plotSettingsWidget->xAxisLabelEdit()->text();
            QString yLabel = plotSettingsWidget->yAxisLabelEdit()->text();
            
            if (!xLabel.isEmpty()) {
                plotTab.plot->xAxis->setLabel(xLabel);
            }
            if (!yLabel.isEmpty()) {
                plotTab.plot->yAxis->setLabel(yLabel);
            }
        }
        plotTab.plot->rescaleAxes();
    }
    plotTab.plot->replot();
}

void MainWindow::draw_line_plot(int first_in, int second_in, int plot_tab_index)
{
    if (m_plotTabs.empty()) {
        return;
    }

    if (plot_tab_index < 0 || plot_tab_index >= m_plotTabs.size()) {
        return;
    }

    PlotTab& plot = m_plotTabs[plot_tab_index];

    Experiment* experiment = Experiment::get_instance();
    if (!experiment) {
        return;
    }

    if (experiment->get_variables_count() == 0) {
        return;
    }

    if (first_in < 0 || second_in < 0 ||
        first_in >= static_cast<int>(experiment->get_variables_count()) ||
        second_in >= static_cast<int>(experiment->get_variables_count())) {
        return;
    }

    Variable& variable_first = experiment->get_variable(first_in);
    Variable& variable_second = experiment->get_variable(second_in);

    // Получаем измерения из выбранных переменных
    const std::vector<double>& xMeasurements = variable_first.get_measurements();
    const std::vector<double>& yMeasurements = variable_second.get_measurements();

    if (xMeasurements.empty() || yMeasurements.empty()) {
        return;
    }

    // Определяем минимальную длину
    size_t minSize = std::min(xMeasurements.size(), yMeasurements.size());

    // Преобразуем в QVector
    QVector<double> xQvector, yQvector;
    xQvector.reserve(static_cast<int>(minSize));
    yQvector.reserve(static_cast<int>(minSize));

    for (size_t i = 0; i < minSize; ++i) {
        xQvector.append(xMeasurements[i]);
        yQvector.append(yMeasurements[i]);
    }

    // Эта функция теперь не используется напрямую, используем draw_single_graph
    // Оставляем для обратной совместимости, но она не должна вызываться
    QCPGraph* graph = nullptr;
    if (plot.plot->graphCount() == 0) {
        graph = plot.plot->addGraph();
    } else {
        graph = plot.plot->graph(0);
    }
    
    draw_single_graph(first_in, second_in, plot_tab_index, graph);
}

void MainWindow::draw_single_graph(int first_in, int second_in, int plot_tab_index, QCPGraph* graph)
{
    if (!graph) {
        return;
    }
    
    if (m_plotTabs.empty()) {
        return;
    }

    if (plot_tab_index < 0 || plot_tab_index >= m_plotTabs.size()) {
        return;
    }

    PlotTab& plot = m_plotTabs[plot_tab_index];

    Experiment* experiment = Experiment::get_instance();
    if (!experiment) {
        return;
    }

    if (experiment->get_variables_count() == 0) {
        return;
    }

    if (first_in < 0 || second_in < 0 ||
        first_in >= static_cast<int>(experiment->get_variables_count()) ||
        second_in >= static_cast<int>(experiment->get_variables_count())) {
        return;
    }

    Variable& variable_first = experiment->get_variable(first_in);
    Variable& variable_second = experiment->get_variable(second_in);

    // Получаем измерения из выбранных переменных
    const std::vector<double>& xMeasurements = variable_first.get_measurements();
    const std::vector<double>& yMeasurements = variable_second.get_measurements();

    if (xMeasurements.empty() || yMeasurements.empty()) {
        return;
    }

    // Определяем минимальную длину
    size_t minSize = std::min(xMeasurements.size(), yMeasurements.size());

    // Преобразуем в QVector
    QVector<double> xQvector, yQvector;
    xQvector.reserve(static_cast<int>(minSize));
    yQvector.reserve(static_cast<int>(minSize));

    for (size_t i = 0; i < minSize; ++i) {
        xQvector.append(xMeasurements[i]);
        yQvector.append(yMeasurements[i]);
    }

    graph->setAdaptiveSampling(false);

    // Применяем настройки из PlotSettingsWidget, если они есть
    // Используем настройки из строки, соответствующей переменной для оси X
    if (plot.settingsTable && plot.settingsTable->rowCount() > first_in) {
        applyPlotSettingsFromTable(graph, plot.settingsTable, first_in);
    } else {
        // Устанавливаем стиль линии по умолчанию
        graph->setLineStyle(QCPGraph::lsLine);
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::blue, Qt::white, 6));
    }

    // Устанавливаем данные
    graph->setData(xQvector, yQvector);

    // Настраиваем оси - используем названия из QLineEdit, если они заданы
    PlotSettingsWidget* plotSettings = qobject_cast<PlotSettingsWidget*>(plot.settingsTab);
    if (plotSettings) {
        QString xLabel = plotSettings->xAxisLabelEdit()->text();
        QString yLabel = plotSettings->yAxisLabelEdit()->text();
        
        // Если названия не заданы, используем названия переменных по умолчанию
        if (xLabel.isEmpty()) {
            xLabel = QString::fromStdString(variable_first.get_name_tables());
            xLabel.remove('\"');
            plotSettings->xAxisLabelEdit()->setText(xLabel);
        }
        if (yLabel.isEmpty()) {
            yLabel = QString::fromStdString(variable_second.get_name_tables());
            yLabel.remove('\"');
            plotSettings->yAxisLabelEdit()->setText(yLabel);
        }
        
        plot.plot->xAxis->setLabel(xLabel);
        plot.plot->yAxis->setLabel(yLabel);
    } else {
        // Fallback на старый способ, если plotSettings недоступен
        plot.plot->xAxis->setLabel(QString::fromStdString(variable_first.get_name_tables()));
        plot.plot->yAxis->setLabel(QString::fromStdString(variable_second.get_name_tables()));
    }

    // Обновляем оси и перерисовываем
    plot.plot->rescaleAxes();
    plot.plot->replot();
}

void MainWindow::on_export_data_triggered()
{
    // Диалог выбора двух файлов: CSV и JSON
    QFileDialog dialog(this);

    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter("*.json *.csv");

    if (dialog.exec() == QFileDialog::Accepted)
    {
        QString csvFile, jsonFile;

        QStringList files = dialog.selectedFiles();
        if (files.size() == 2)
        {
            for (const QString &filePath : files)
            {
                if (filePath.endsWith(".csv", Qt::CaseInsensitive))
                {
                    csvFile = filePath;
                }
                else if (filePath.endsWith(".json", Qt::CaseInsensitive))
                {
                    jsonFile = filePath;
                }
            }

            // Проверяем, что CSV-файл действительно выбран
            if (csvFile.isEmpty())
            {
                QMessageBox::critical(this,
                                      "Ошибка выбора файлов",
                                      "Не выбран CSV-файл для сохранения данных.");
                return;
            }

            Experiment *experiment = Experiment::get_instance();

            size_t num_of_variables = experiment->get_variables_count();
            if (!num_of_variables)
            {
                QMessageBox::critical(this,
                                      "Ошибка сохранения данных",
                                      "Пожалуйста, убедитесь, что таблица непустая.");
                return;
            }

            // Получаем количество измерений только после проверки, что есть переменные
            size_t num_of_measurements = experiment->get_variable(0).get_measurements_count();
            if (num_of_measurements == 0) {
                QMessageBox::critical(this,
                                      "Ошибка сохранения данных",
                                      "В таблице нет ни одного измерения. CSV не будет перезаписан нулями.");
                return;
            }

            QFile file(csvFile);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::critical(this,
                                      "Ошибка сохранения данных",
                                      "Не удалось открыть CSV-файл для записи.");
                return;
            }

            QTextStream stream(&file);

            // Заполняем CSV-файл именами переменных
            for (size_t curr_num_of_variable = 0; curr_num_of_variable < num_of_variables; ++curr_num_of_variable)
            {
                stream << QString::fromStdString(experiment->get_variable(curr_num_of_variable).get_name_tables());
                if (curr_num_of_variable != (num_of_variables - 1))
                {
                    stream << ",";
                }
                else
                {
                    stream << "\n";
                }
            }

            // Заполняем CSV-файл измерениями
            for (size_t curr_num_of_measurement = 0; curr_num_of_measurement < num_of_measurements; ++curr_num_of_measurement)
            {
                for (size_t curr_num_of_variable = 0; curr_num_of_variable < num_of_variables; ++curr_num_of_variable)
                {
                    stream << QString::number(experiment->get_variable(curr_num_of_variable).get_measurement(curr_num_of_measurement));
                    if (curr_num_of_variable != (num_of_variables - 1))
                    {
                        stream << ",";
                    }
                    else
                    {
                        stream << "\n";
                    }
                }
            }
            file.close();

                // Сохранение JSON файла, если он выбран
            if (!jsonFile.isEmpty())
            {
                using json = nlohmann::json;
                json json_data;

                // Собираем уникальные инструменты
                std::map<std::string, std::pair<std::string, double>> instruments_map; // name -> (type, error)
                json variables_json = json::object(); // Явно инициализируем как объект, а не null

                for (size_t curr_num_of_variable = 0; curr_num_of_variable < num_of_variables; ++curr_num_of_variable)
                {
                    Variable& variable = experiment->get_variable(curr_num_of_variable);
                    std::string variable_name = variable.get_name_tables();
                    Instrument* instrument = variable.get_instrument();

                    if (instrument != nullptr)
                    {
                        std::string instrument_name = instrument->get_name();
                        
                        // Пропускаем инструмент "(нет инструмента)"
                        if (instrument_name != "(нет инструмента)")
                        {
                            // Определяем тип инструмента и получаем значение погрешности
                            std::string error_type;
                            double error_value = 0.0;

                            if (AbsoluteInstrument* abs_instr = dynamic_cast<AbsoluteInstrument*>(instrument))
                            {
                                error_type = "Absolute";
                                error_value = abs_instr->get_error(0, 0);
                            }
                            else if (RelativeInstrument* rel_instr = dynamic_cast<RelativeInstrument*>(instrument))
                            {
                                error_type = "Relative";
                                error_value = rel_instr->get_error(0, 1.0); // Для относительной погрешности используем значение при value=1.0
                            }

                            // Добавляем инструмент в map только если тип определен (Absolute или Relative)
                            if (!error_type.empty())
                            {
                                // Добавляем инструмент в map, если его еще нет
                                if (instruments_map.find(instrument_name) == instruments_map.end())
                                {
                                    instruments_map[instrument_name] = std::make_pair(error_type, error_value);
                                }

                                // Добавляем связь переменной с инструментом
                                variables_json[variable_name] = instrument_name;
                            }
                        }
                    }
                }

                // Формируем секцию Instruments
                json instruments_json = json::object(); // Явно инициализируем как объект, а не null
                for (const auto& [name, type_error] : instruments_map)
                {
                    instruments_json[name]["type"] = type_error.first;
                    instruments_json[name]["error"] = type_error.second;
                }

                json_data["Instruments"] = instruments_json;
                json_data["Variables"] = variables_json;

                // Сохраняем JSON в файл с правильным форматированием
                QFile json_file(jsonFile);
                if (json_file.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    QTextStream json_stream(&json_file);
                    // Используем dump(2) для форматирования с отступами в 2 пробела
                    std::string json_string = json_data.dump(2);
                    // Заменяем форматирование на точное соответствие примеру (пробелы вокруг двоеточий)
                    json_stream << QString::fromStdString(json_string);
                    json_file.close();
                }
                else
                {
                    QMessageBox::warning(this,
                                        "Предупреждение",
                                        "CSV файл сохранен, но не удалось открыть JSON-файл для записи.");
                }
            }

            QMessageBox::information(this, "Успешно","Данные успешно экспортированы.");
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
