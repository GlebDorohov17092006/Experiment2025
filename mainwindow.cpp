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

    ui->tableViewMeasurements->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableViewMeasurements->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableViewInstruments->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableViewInstruments->setSelectionBehavior(QAbstractItemView::SelectRows);

    setupNoInstrument();

    Experiment::destroy_instance();
    Experiment::get_instance(std::vector<Variable>(), std::vector<Variable>());

    ui->tableViewMeasurements->setModel(m_tableModel);
    ui->tableViewMeasurements->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableViewInstruments->setModel(m_instrumentsModel);
    ui->tableViewInstruments->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_instrumentsModel->setInstruments(&m_instruments);

    updateInstrumentDelegate();
    setupErrorTypeDelegate();

    ui->variableInstrumentsTable->setItemDelegateForRow(0, m_instrumentDelegate);
    ui->tableViewInstruments->setItemDelegateForColumn(1, m_errorTypeDelegate);

    // УДАЛИЛ ВСЕ КНОПКИ ЗДЕСЬ - они уже подключены в Designer

    connect(ui->variableInstrumentsTable, &QTableWidget::itemChanged, this, &MainWindow::onInstrumentChanged);
    connect(m_instrumentsModel, &InstrumentsModel::instrumentNameChanged, this, &MainWindow::updateInstrumentDelegate);

    connect(ui->add_graph, &QAction::triggered, this, [this]() { addDynamicPlotTab("График"); });
    connect(ui->add_histogram, &QAction::triggered, this, [this]() { addDynamicPlotTab("Гистограмма"); });
    connect(ui->add_scatterplot, &QAction::triggered, this, [this]() { addDynamicPlotTab("Скаттерплот"); });
    connect(ui->delete_plot, &QAction::triggered, this, [this]() { removeGraph(); });

    connect(ui->tabPlot->tabBar(), &QTabBar::tabMoved, this, &MainWindow::onPlotTabMoved);

    ui->tableViewInstruments->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableViewInstruments->setColumnWidth(0, 150);
    ui->tableViewInstruments->setColumnWidth(1, 300);
    ui->tableViewInstruments->setColumnWidth(2, 100);

    ui->variableInstrumentsTable->horizontalHeader()->setDefaultSectionSize(200);

    addDynamicPlotTab("График");
    addDynamicPlotTab("Гистограмма");
    addDynamicPlotTab("Скаттерплот");

    connect(ui->tableViewMeasurements->horizontalHeader(), &QHeaderView::sectionDoubleClicked,
            this, &MainWindow::onColumnHeaderDoubleClicked);

    connect(ui->variableInstrumentsTable, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::onInstrumentCellDoubleClicked);

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
    }
    ui->variableInstrumentsTable->setHorizontalHeaderLabels(headers);

    for (size_t i = 0; i < variablesCount; ++i) {
        auto& var = experiment->get_variable(i);

        QTableWidgetItem* item = new QTableWidgetItem();
        QString instrumentName = QString::fromStdString(var.get_name_instrument());
        item->setText(instrumentName);
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        ui->variableInstrumentsTable->setItem(0, static_cast<int>(i), item);
    }

    ui->variableInstrumentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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

    // Получаем текущий выбранный столбец в таблице измерений
    QModelIndexList selectedIndexes = ui->tableViewMeasurements->selectionModel()->selectedIndexes();

    int columnToRemove = -1;
    if (!selectedIndexes.isEmpty()) {
        // Берем первый выбранный индекс
        columnToRemove = selectedIndexes.first().column();
    } else {
        // Если ничего не выбрано, удаляем последний (старое поведение)
        columnToRemove = static_cast<int>(experiment->get_variables_count()) - 1;
    }

    if (columnToRemove < 0 || columnToRemove >= static_cast<int>(experiment->get_variables_count())) {
        QMessageBox::warning(this, "Ошибка", "Некорректный номер столбца");
        return;
    }

    // Подтверждение удаления
    QString columnName = getColumnName(columnToRemove);
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение удаления",
        QString("Вы действительно хотите удалить столбец '%1'?").arg(columnName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        experiment->remove_variable(columnToRemove);
        m_tableModel->refreshData();
        updateVariableInstrumentsTable();
        syncPlotSettingsTables();
    }
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

    // Находим максимальное количество строк
    size_t maxRows = 0;
    for (size_t i = 0; i < experiment->get_variables_count(); ++i) {
        auto& var = experiment->get_variable(i);
        maxRows = std::max(maxRows, var.get_measurements_count());
    }

    if (maxRows == 0) {
        QMessageBox::information(this, "Информация", "Нет строк для удаления");
        return;
    }

    // Получаем текущую выбранную строку в таблице измерений
    QModelIndexList selectedIndexes = ui->tableViewMeasurements->selectionModel()->selectedIndexes();

    int rowToRemove = -1;
    if (!selectedIndexes.isEmpty()) {
        // Берем первую выбранную строку
        rowToRemove = selectedIndexes.first().row();
    } else {
        // Если ничего не выбрано, удаляем последнюю (старое поведение)
        rowToRemove = static_cast<int>(maxRows) - 1;
    }

    if (rowToRemove < 0 || rowToRemove >= static_cast<int>(maxRows)) {
        QMessageBox::warning(this, "Ошибка", "Некорректный номер строки");
        return;
    }

    // Подтверждение удаления
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение удаления",
        QString("Вы действительно хотите удалить строку %1?").arg(rowToRemove + 1),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (size_t i = 0; i < experiment->get_variables_count(); ++i) {
            auto& var = experiment->get_variable(i);
            if (var.get_measurements_count() > rowToRemove) {
                var.remove_measurement(rowToRemove);
            }
        }
        m_tableModel->refreshData();
    }
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

    // Получаем текущий выбранный инструмент в таблице инструментов
    QModelIndexList selectedIndexes = ui->tableViewInstruments->selectionModel()->selectedIndexes();

    int rowToRemove = -1;
    if (!selectedIndexes.isEmpty()) {
        // Берем первую выбранную строку
        rowToRemove = selectedIndexes.first().row();

        // Проверяем, что это не инструмент "Нет инструмента"
        if (rowToRemove == 0) {
            QMessageBox::information(this, "Информация", "Нельзя удалить базовый инструмент 'Нет инструмента'");
            return;
        }
    } else {
        // Если ничего не выбрано, удаляем последний (старое поведение)
        rowToRemove = static_cast<int>(m_instruments.size()) - 1;

        // Проверяем, что это не инструмент "Нет инструмента"
        if (rowToRemove == 0) {
            QMessageBox::information(this, "Информация", "Нельзя удалить базовый инструмент 'Нет инструмента'");
            return;
        }
    }

    if (rowToRemove <= 0 || rowToRemove >= static_cast<int>(m_instruments.size())) {
        QMessageBox::warning(this, "Ошибка", "Некорректный номер инструмента");
        return;
    }

    // Получаем имя удаляемого инструмента
    auto instrumentToRemove = m_instruments[rowToRemove];
    QString instrumentName = QString::fromStdString(instrumentToRemove->get_name());

    // Подтверждение удаления
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение удаления",
        QString("Вы действительно хотите удалить инструмент '%1'?").arg(instrumentName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        auto experiment = Experiment::get_instance();

        // Заменяем удаляемый инструмент на "Нет инструмента" во всех переменных
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

        // Удаляем инструмент из списка
        m_instruments.erase(m_instruments.begin() + rowToRemove);

        // Обновляем модели
        m_instrumentsModel->refreshData();
        updateInstrumentDelegate();
        m_tableModel->refreshData();
    }
}

void MainWindow::onPlotSettingsTabChanged(int index)
{
    ui->tabPlot->blockSignals(true);
    ui->tabPlot->setCurrentIndex(index);
    ui->tabPlot->blockSignals(false);
}

void MainWindow::onPlotTabChanged(int index)
{
    ui->tabPlotSettings->blockSignals(true);
    ui->tabPlotSettings->setCurrentIndex(index);
    ui->tabPlotSettings->blockSignals(false);
}

void MainWindow::onPlotTabMoved(int from, int to)
{
    PlotTab movedTab = m_plotTabs[from];
    m_plotTabs.removeAt(from);
    m_plotTabs.insert(to, movedTab);

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

            updateVariableInstrumentsTable();

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

    for (auto& plotTab : m_plotTabs) {
        if (!plotTab.settingsTable) {
            continue;
        }

        int currentRows = plotTab.settingsTable->rowCount();

        if (currentRows < mainTableColumns) {
            for (int i = currentRows; i < mainTableColumns; ++i) {
                QString columnName = getColumnName(i);
                plotTab.settingsTable->insertRow(i);
                plotTab.settingsTable->setVerticalHeaderItem(i, new QTableWidgetItem(columnName));

                if (plotTab.type == "График") {
                    QTableWidgetItem* checkItem = new QTableWidgetItem();
                    checkItem->setCheckState(Qt::Checked);
                    plotTab.settingsTable->setItem(i, 0, checkItem);

                    plotTab.settingsTable->setItem(i, 1, new QTableWidgetItem("Сплошная"));
                    plotTab.settingsTable->setItem(i, 2, new QTableWidgetItem("1"));
                    plotTab.settingsTable->setItem(i, 3, new QTableWidgetItem("Круг"));
                    plotTab.settingsTable->setItem(i, 4, new QTableWidgetItem("6"));

                    QColor defaultColor = QColor::fromHsv((i * 60) % 360, 255, 255);
                    plotTab.settingsTable->setItem(i, 5, new QTableWidgetItem(defaultColor.name()));
                }
            }
        }
        else if (currentRows > mainTableColumns) {
            for (int i = currentRows - 1; i >= mainTableColumns; --i) {
                plotTab.settingsTable->removeRow(i);
            }
        }

        for (int i = 0; i < mainTableColumns; ++i) {
            QString columnName = getColumnName(i);
            plotTab.settingsTable->setVerticalHeaderItem(i, new QTableWidgetItem(columnName));
        }
    }
}

void MainWindow::onInstrumentCellDoubleClicked(int row, int column)
{
    if (row != 0) return;

    QTableWidgetItem* item = ui->variableInstrumentsTable->item(row, column);
    if (!item) return;

    ui->variableInstrumentsTable->editItem(item);
}

void MainWindow::addDynamicPlotTab(const QString& plotType)
{
    QString baseName = plotType;
    QString tabName;

    bool hasBaseName = false;
    for (const auto& tab : m_plotTabs) {
        if (tab.name == baseName) {
            hasBaseName = true;
            break;
        }
    }

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

    QWidget* plotWidget = new QWidget();
    QGridLayout* plotLayout = new QGridLayout(plotWidget);
    QCustomPlot* plot = new QCustomPlot(plotWidget);
    plot->setInteraction(QCP::iRangeDrag, true);
    plot->setInteraction(QCP::iRangeZoom, true);
    plotLayout->addWidget(plot, 0, 0);

    ui->tabPlot->addTab(plotWidget, tabName);

    BaseSettingsWidget* settingsWidget = BaseSettingsWidget::create(plotType);
    if (!settingsWidget) {
        return;
    }

    QTableWidget* settingsTable = settingsWidget->settingsTable();
    if (!settingsTable) {
        delete settingsWidget;
        return;
    }

    if (plotType == "График") {
        PlotSettingsWidget* plotSettings = qobject_cast<PlotSettingsWidget*>(settingsWidget);
        if (plotSettings) {
            settingsWidget->setupDelegates(this);

            int rowIndex = settingsTable->rowCount();
            settingsTable->insertRow(rowIndex);

            QTableWidgetItem* enabledItem = new QTableWidgetItem();
            enabledItem->setCheckState(Qt::Checked);
            settingsTable->setItem(rowIndex, PlotSettingsWidget::ColumnEnabled, enabledItem);

            QTableWidgetItem* lineTypeItem = new QTableWidgetItem("Сплошная");
            settingsTable->setItem(rowIndex, PlotSettingsWidget::ColumnLineType, lineTypeItem);

            QTableWidgetItem* widthItem = new QTableWidgetItem("1");
            settingsTable->setItem(rowIndex, PlotSettingsWidget::ColumnWidth, widthItem);

            QTableWidgetItem* pointTypeItem = new QTableWidgetItem("Без точки");
            settingsTable->setItem(rowIndex, PlotSettingsWidget::ColumnPointType, pointTypeItem);

            QTableWidgetItem* pointSizeItem = new QTableWidgetItem("6");
            settingsTable->setItem(rowIndex, PlotSettingsWidget::ColumnPointSize, pointSizeItem);

            QTableWidgetItem* colorItem = new QTableWidgetItem("#0000ff");
            settingsTable->setItem(rowIndex, PlotSettingsWidget::ColumnColor, colorItem);

            updateVariableComboBoxes(plotSettings);
        }
    }

    ui->tabPlotSettings->addTab(settingsWidget, tabName);

    PlotTab plotTab;
    plotTab.name = tabName;
    plotTab.type = plotType;
    plotTab.plot = plot;
    plotTab.settingsTab = settingsWidget;
    plotTab.settingsTable = settingsTable;
    m_plotTabs.append(plotTab);

    if (plotType == "График") {
        PlotSettingsWidget* plotSettings = qobject_cast<PlotSettingsWidget*>(settingsWidget);
        if (plotSettings) {
            int tabIndex = m_plotTabs.size() - 1;

            connect(settingsTable, &QTableWidget::cellChanged, this, [this, tabIndex](int row, int column) {
                Q_UNUSED(column);
                if (tabIndex >= 0 && tabIndex < m_plotTabs.size()) {
                    PlotTab& plotTab = m_plotTabs[tabIndex];
                    if (plotTab.plot->graphCount() > 0 && row >= 0) {
                        QCPGraph* graph = plotTab.plot->graph(0);
                        PlotSettingsWidget* plotSettings = qobject_cast<PlotSettingsWidget*>(plotTab.settingsTab);
                        if (plotSettings) {
                            applyPlotSettingsFromTable(graph, plotSettings->settingsTable(), row);
                            plotTab.plot->replot();
                        }
                    }
                }
            });

            QComboBox* xAxisCombo = plotSettings->xAxisComboBox();
            QComboBox* yAxisCombo = plotSettings->yAxisComboBox();

            connect(xAxisCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, tabIndex, plotSettings]() {
                if (tabIndex >= 0 && tabIndex < m_plotTabs.size()) {
                    int xIndex = plotSettings->xAxisComboBox()->currentData().toInt();
                    int yIndex = plotSettings->yAxisComboBox()->currentData().toInt();
                    draw_line_plot(xIndex, yIndex, tabIndex);
                }
            });

            connect(yAxisCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, tabIndex, plotSettings]() {
                if (tabIndex >= 0 && tabIndex < m_plotTabs.size()) {
                    int xIndex = plotSettings->xAxisComboBox()->currentData().toInt();
                    int yIndex = plotSettings->yAxisComboBox()->currentData().toInt();
                    draw_line_plot(xIndex, yIndex, tabIndex);
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
    QFileDialog dialog(this);

    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter("*.json *.csv");

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

            if(csvFile.isEmpty() || jsonFile.isEmpty()) {
                QMessageBox::critical(this, "Ошибка",
                    "Необходимо выбрать один CSV и один JSON файл");
                return;
            }

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

            if (variables.empty()) {
                QApplication::restoreOverrideCursor();
                QMessageBox::warning(this, "Предупреждение",
                    "Не удалось загрузить данные из файлов.");
                return;
            }

            Experiment::destroy_instance();

            Experiment* experiment = Experiment::get_instance(variables, std::vector<Variable>());

            QApplication::processEvents();

            m_tableModel->refreshData();

            updateVariableInstrumentsTable();

            for (PlotTab& plotTab : m_plotTabs) {
                if (plotTab.type == "График") {
                    PlotSettingsWidget* plotSettings = qobject_cast<PlotSettingsWidget*>(plotTab.settingsTab);
                    if (plotSettings) {
                        updateVariableComboBoxes(plotSettings);
                    }
                }
            }

            QApplication::restoreOverrideCursor();
            QMessageBox::information(this, "Успешно","Файлы успешно загружены и обработаны.");
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

    QComboBox* xAxisCombo = plotSettings->xAxisComboBox();
    QComboBox* yAxisCombo = plotSettings->yAxisComboBox();

    if (!xAxisCombo || !yAxisCombo) {
        return;
    }

    int currentXIndex = -1;
    int currentYIndex = -1;
    if (xAxisCombo->count() > 0) {
        currentXIndex = xAxisCombo->currentData().toInt();
    }
    if (yAxisCombo->count() > 0) {
        currentYIndex = yAxisCombo->currentData().toInt();
    }

    xAxisCombo->clear();
    yAxisCombo->clear();

    if (experiment->get_variables_count() == 0) {
        return;
    }

    for (size_t i = 0; i < experiment->get_variables_count(); ++i) {
        auto variable = experiment->get_variable(i);
        QString varName = QString::fromStdString(variable.get_name_tables());
        if (varName.isEmpty()) {
            varName = QString("Переменная %1").arg(i + 1);
        }
        xAxisCombo->addItem(varName, static_cast<int>(i));
        yAxisCombo->addItem(varName, static_cast<int>(i));
    }

    if (currentXIndex < 0 || currentXIndex >= static_cast<int>(experiment->get_variables_count())) {
        currentXIndex = 0;
    }
    if (currentYIndex < 0 || currentYIndex >= static_cast<int>(experiment->get_variables_count())) {
        currentYIndex = (experiment->get_variables_count() > 1) ? 1 : 0;
    }

    int xComboIndex = xAxisCombo->findData(currentXIndex);
    if (xComboIndex >= 0) {
        xAxisCombo->setCurrentIndex(xComboIndex);
    } else if (xAxisCombo->count() > 0) {
        xAxisCombo->setCurrentIndex(0);
    }

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

    QTableWidgetItem* enabledItem = settingsTable->item(rowIndex, PlotSettingsWidget::ColumnEnabled);
    if (enabledItem && enabledItem->checkState() != Qt::Checked) {
        graph->setVisible(false);
        return;
    }
    graph->setVisible(true);

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

    QTableWidgetItem* pointSizeItem = settingsTable->item(rowIndex, PlotSettingsWidget::ColumnPointSize);
    if (pointSizeItem) {
        bool ok;
        double size = pointSizeItem->text().toDouble(&ok);
        if (ok && size > 0) {
            scatterStyle.setSize(size);
        }
    }

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

    const std::vector<double>& xMeasurements = variable_first.get_measurements();
    const std::vector<double>& yMeasurements = variable_second.get_measurements();

    if (xMeasurements.empty() || yMeasurements.empty()) {
        return;
    }

    size_t minSize = std::min(xMeasurements.size(), yMeasurements.size());

    QVector<double> xQvector, yQvector;
    xQvector.reserve(static_cast<int>(minSize));
    yQvector.reserve(static_cast<int>(minSize));

    for (size_t i = 0; i < minSize; ++i) {
        xQvector.append(xMeasurements[i]);
        yQvector.append(yMeasurements[i]);
    }

    if (plot.plot->graphCount() == 0) {
        plot.plot->addGraph();
    }

    QCPGraph* graph = plot.plot->graph(0);

    graph->setAdaptiveSampling(false);

    if (plot.settingsTable && plot.settingsTable->rowCount() > 0) {
        applyPlotSettingsFromTable(graph, plot.settingsTable, 0);
    } else {
        graph->setLineStyle(QCPGraph::lsLine);
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::blue, Qt::white, 6));
    }

    graph->setData(xQvector, yQvector);

    plot.plot->xAxis->setLabel(QString::fromStdString(variable_first.get_name_tables()));
    plot.plot->yAxis->setLabel(QString::fromStdString(variable_second.get_name_tables()));

    plot.plot->rescaleAxes();
    plot.plot->replot();
}

void MainWindow::on_export_data_triggered()
{
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
