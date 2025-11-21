#include <windows.h>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "parser.h"
#include "qcustomplot.h"
#include "basesettingswidget.h"
#include "Experiment.h"
#include "TableModel.h"
#include "InstrumentsModel.h"
#include "Variable.h"
#include "AbsoluteInstrument.h"
#include "RelativeInstrument.h"
#include "CombinedInstrument.h"

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
#include <algorithm>
#include <QFileDialog>
#include <QTextEdit>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tableModel(new TableModel(this))
    , m_instrumentsModel(new InstrumentsModel(this))
    , m_experiment(nullptr)
{
    ui->setupUi(this);

    // Привязываем модели
    m_tableModel->setExperiment(m_experiment);
    m_instrumentsModel->setInstruments(&m_instruments);

    // ЗАМЕНЯЕМ tableWidget на TableView
    QTableView* measurementsView = ui->tableWidget;
    measurementsView->setModel(m_tableModel);
    measurementsView->setSelectionBehavior(QAbstractItemView::SelectItems);
    measurementsView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    measurementsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // ЗАМЕНЯЕМ instrumentsTable на TableView
    QTableView* instrumentsView = ui->instrumentsTable;
    instrumentsView->setModel(m_instrumentsModel);
    instrumentsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    instrumentsView->verticalHeader()->setVisible(false);

    connect(ui->addColumnButton, &QPushButton::clicked, this, &MainWindow::addColumn);
    connect(ui->removeColumnButton, &QPushButton::clicked, this, &MainWindow::removeColumn);
    connect(ui->addRowButton, &QPushButton::clicked, this, &MainWindow::addRow);
    connect(ui->removeRowButton, &QPushButton::clicked, this, &MainWindow::removeRow);
    connect(ui->addInstrument, &QPushButton::clicked, this, &MainWindow::addInstrument);
    connect(ui->deleteInstrument, &QPushButton::clicked, this, &MainWindow::removeInstrument);

    ui->instrumentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->instrumentsTable->setColumnWidth(0, 150);
    ui->instrumentsTable->setColumnWidth(1, 300);
    ui->instrumentsTable->setColumnWidth(2, 100);

    connect(ui->add_graph, &QAction::triggered, this, [this]() { addDynamicPlotTab("График"); });
    connect(ui->add_histogram, &QAction::triggered, this, [this]() { addDynamicPlotTab("Гистограмма"); });
    connect(ui->add_scatterplot, &QAction::triggered, this, [this]() { addDynamicPlotTab("Скаттерплот"); });
    connect(ui->delete_plot, &QAction::triggered, this, [this]() { removeGraph(); });

    // Подключаем обработчик для перетаскивания вкладок графиков
    connect(ui->tabPlot->tabBar(), &QTabBar::tabMoved, this, &MainWindow::onPlotTabMoved);


    ui->variableInstrumentsTable->horizontalHeader()->setDefaultSectionSize(200);

    //syncVariableInstrumentsTable();

    addDynamicPlotTab("График");
    addDynamicPlotTab("Гистограмма");
    addDynamicPlotTab("Скаттерплот");
}

MainWindow::~MainWindow()
{
    delete ui;
}


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

void MainWindow::addColumn()
{
//    int currentColumns = ui->tableWidget->columnCount();
//
//    ui->tableWidget->insertColumn(currentColumns);
//
//    QString headerName = QString("Столбец %1").arg(currentColumns + 1);
//    ui->tableWidget->setHorizontalHeaderItem(currentColumns, new QTableWidgetItem(headerName));
//
//
//    ui->variableInstrumentsTable->insertColumn(currentColumns);
//    ui->variableInstrumentsTable->setHorizontalHeaderItem(currentColumns, new QTableWidgetItem(headerName));
//
//    QTableWidgetItem* item = new QTableWidgetItem("-");
//    item->setData(Qt::UserRole, -1); // -1 означает, что инструмент не выбран
//    ui->variableInstrumentsTable->setItem(0, currentColumns, item);
//
//    // Добавляем строку в таблицы настроек динамически добавленных графиков
//    for (auto& plotTab : m_plotTabs) {
//        int rowIndex = plotTab.settingsTable->rowCount();
//        plotTab.settingsTable->insertRow(rowIndex);
//        plotTab.settingsTable->setVerticalHeaderItem(rowIndex, new QTableWidgetItem(headerName));
//
//        QTableWidgetItem* checkItem = new QTableWidgetItem();
//        checkItem->setCheckState(Qt::Checked);
//        plotTab.settingsTable->setItem(rowIndex, BaseSettingsWidget::ColumnEnabled, checkItem);
//
//        // Добавляем остальные колонки
//        int columnCount = plotTab.settingsTable->columnCount();
//        for (int j = 1; j < columnCount; ++j) {
//            plotTab.settingsTable->setItem(rowIndex, j, new QTableWidgetItem(""));
//        }
//    }
}

void MainWindow::onColumnHeaderDoubleClicked(int col)
{
//    QString oldName = getColumnName(col);
//    QString oldTag = getColumnTag(col);
//
//    QDialog dialog(this);
//    dialog.setWindowTitle("Настройка столбца");
//
//    QFormLayout *formLayout = new QFormLayout(&dialog);
//
//    QLineEdit *nameEdit = new QLineEdit(oldName, &dialog);
//    formLayout->addRow("Полное имя:", nameEdit);
//
//    QLineEdit *tagEdit = new QLineEdit(oldTag, &dialog);
//    QLabel *tagHint = new QLabel("", &dialog);
//    tagHint->setStyleSheet("color: gray; font-size: 9pt;");
//    formLayout->addRow("Тег:", tagEdit);
//    formLayout->addRow("", tagHint);
//
//    QDialogButtonBox *buttonBox = new QDialogButtonBox(
//        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
//    formLayout->addRow(buttonBox);
//
//    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
//    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
//
//    if (dialog.exec() == QDialog::Accepted) {
//        QString newName = nameEdit->text().trimmed();
//        QString newTag = tagEdit->text().trimmed();
//
//        if (!newName.isEmpty()) {
//            setColumnTag(col, newTag);
//
//            QString headerText = newName;
//            if (!newTag.isEmpty()) {
//                headerText += "\n(" + newTag + ")";
//            }
//
//            ui->tableWidget->setHorizontalHeaderItem(col, new QTableWidgetItem(headerText));
//
//            // Обновляем вертикальный заголовок в таблицах настроек динамически добавленных графиков
//            for (auto& plotTab : m_plotTabs) {
//                if (col < plotTab.settingsTable->rowCount()) {
//                    plotTab.settingsTable->setVerticalHeaderItem(col, new QTableWidgetItem(newName));
//                }
//            }
//        }
//    }
}

QString MainWindow::getColumnName(int columnIndex)
{
//    QTableWidgetItem* headerItem = ui->tableWidget->horizontalHeaderItem(columnIndex);
//    if (headerItem) {
//        QString fullText = headerItem->text();
//        int newlinePos = fullText.indexOf('\n');
//        if (newlinePos != -1) {
//            return fullText.left(newlinePos);
//        }
//        return fullText;
//    } else {
//        return QString("Столбец %1").arg(columnIndex + 1);
//    }
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

void MainWindow::saveReport()
{

}

void MainWindow::updateVariableInstrumentsTable()
{
//    int mainTableColumns = ui->tableWidget->columnCount();
//    int instrumentTableColumns = ui->variableInstrumentsTable->columnCount();
//
//    // Синхронизируем количество столбцов
//    while (instrumentTableColumns < mainTableColumns) {
//        ui->variableInstrumentsTable->insertColumn(instrumentTableColumns);
//        QString columnName = getColumnName(instrumentTableColumns);
//        ui->variableInstrumentsTable->setHorizontalHeaderItem(instrumentTableColumns, new QTableWidgetItem(columnName));
//
//        // Создаем ячейку с текстом вместо ComboBox
//        QTableWidgetItem* item = new QTableWidgetItem("-");
//        item->setData(Qt::UserRole, -1); // -1 означает, что инструмент не выбран
//        ui->variableInstrumentsTable->setItem(0, instrumentTableColumns, item);
//
//        instrumentTableColumns++;
//    }
//
//    // Обновляем заголовки столбцов
//    for (int i = 0; i < mainTableColumns && i < ui->variableInstrumentsTable->columnCount(); ++i) {
//        QString columnName = getColumnName(i);
//        ui->variableInstrumentsTable->setHorizontalHeaderItem(i, new QTableWidgetItem(columnName));
//    }
//
//    // Синхронизируем таблицы настроек графиков
//    syncPlotSettingsTables();
}

void MainWindow::syncPlotSettingsTables()
{
//    int mainTableColumns = ui->tableWidget->columnCount();
//
//    // Синхронизируем таблицы настроек динамически добавленных графиков
//    for (auto& plotTab : m_plotTabs) {
//        // Добавляем недостающие строки
//        while (plotTab.settingsTable->rowCount() < mainTableColumns) {
//            QString columnName = getColumnName(plotTab.settingsTable->rowCount());
//            int rowIndex = plotTab.settingsTable->rowCount();
//            plotTab.settingsTable->insertRow(rowIndex);
//            plotTab.settingsTable->setVerticalHeaderItem(rowIndex, new QTableWidgetItem(columnName));
//
//            QTableWidgetItem* checkItem = new QTableWidgetItem();
//            checkItem->setCheckState(Qt::Checked);
//            plotTab.settingsTable->setItem(rowIndex, BaseSettingsWidget::ColumnEnabled, checkItem);
//
//            // Добавляем остальные колонки
//            int columnCount = plotTab.settingsTable->columnCount();
//            for (int j = 1; j < columnCount; ++j) {
//                plotTab.settingsTable->setItem(rowIndex, j, new QTableWidgetItem(""));
//            }
//        }
//
//        // Обновляем вертикальные заголовки
//        for (int i = 0; i < mainTableColumns; ++i) {
//            QString columnName = getColumnName(i);
//            if (i < plotTab.settingsTable->rowCount()) {
//                plotTab.settingsTable->setVerticalHeaderItem(i, new QTableWidgetItem(columnName));
//            }
//        }
//    }
}

void MainWindow::updateInstrumentTexts()
{
//    // Обновляем текст во всех ячейках таблицы инструментов
//    int columnCount = ui->variableInstrumentsTable->columnCount();
//
//    for (int col = 0; col < columnCount; ++col) {
//        QTableWidgetItem* item = ui->variableInstrumentsTable->item(0, col);
//        if (item) {
//            int instrumentIndex = item->data(Qt::UserRole).toInt();
//            QString displayText = getInstrumentDisplayText(instrumentIndex);
//            item->setText(displayText);
//        }
//    }
}

void MainWindow::onInstrumentCellDoubleClicked(int row, int column)
{
//    if (row != 0) return; // У нас только одна строка
//
//    QTableWidgetItem* item = ui->variableInstrumentsTable->item(row, column);
//    if (!item) return;
//
//    // Сохраняем текущий выбор
//    int currentInstrument = item->data(Qt::UserRole).toInt();
//
//    // Создаем временный ComboBox
//    QComboBox* comboBox = new QComboBox(ui->variableInstrumentsTable);
//    comboBox->addItem("-", -1);
//
//    // Заполняем список инструментами
//    int instrumentCount = ui->instrumentsTable->rowCount();
//    for (int i = 0; i < instrumentCount; ++i) {
//        QString displayText = getInstrumentDisplayText(i);
//        if (displayText != "-") {
//            comboBox->addItem(displayText, i);
//        }
//    }
//
//    // Устанавливаем текущий выбор
//    int comboIndex = comboBox->findData(currentInstrument);
//    if (comboIndex >= 0) {
//        comboBox->setCurrentIndex(comboIndex);
//    }
//
//    // Показываем ComboBox
//    ui->variableInstrumentsTable->setCellWidget(row, column, comboBox);
//    comboBox->showPopup();
//
//    // Обработчик выбора значения
//    connect(comboBox, QOverload<int>::of(&QComboBox::activated), this,
//            [this, row, column, comboBox]() {
//                int selectedInstrument = comboBox->currentData().toInt();
//
//                // Создаем или обновляем ячейку
//                QTableWidgetItem* item = ui->variableInstrumentsTable->item(row, column);
//                if (!item) {
//                    item = new QTableWidgetItem();
//                    ui->variableInstrumentsTable->setItem(row, column, item);
//                }
//
//                // Сохраняем выбранный инструмент
//                item->setData(Qt::UserRole, selectedInstrument);
//
//                // Обновляем текст
//                QString displayText = getInstrumentDisplayText(selectedInstrument);
//                item->setText(displayText);
//
//                // Удаляем ComboBox
//                ui->variableInstrumentsTable->removeCellWidget(row, column);
//                comboBox->deleteLater();
//            });
//
//    // Обработчик закрытия без выбора
//    connect(comboBox, &QComboBox::destroyed, this, [this, row, column]() {
//        ui->variableInstrumentsTable->removeCellWidget(row, column);
//    });
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

//    // Настраиваем делегаты для редактирования ячеек
//    settingsWidget->setupDelegates(this);
//
//    int columnCount = ui->tableWidget->columnCount();
//    int tableColumnCount = settingsTable->columnCount();
//
//    for (int i = 0; i < columnCount; ++i) {
//        QString columnName = getColumnName(i);
//        int rowIndex = settingsTable->rowCount();
//        settingsTable->insertRow(rowIndex);
//        settingsTable->setVerticalHeaderItem(rowIndex, new QTableWidgetItem(columnName));
//
//        QTableWidgetItem* checkItem = new QTableWidgetItem();
//        checkItem->setCheckState(Qt::Checked);
//        settingsTable->setItem(rowIndex, BaseSettingsWidget::ColumnEnabled, checkItem);
//
//        for (int j = 1; j < tableColumnCount; ++j) {
//            settingsTable->setItem(rowIndex, j, new QTableWidgetItem(""));
//        }
//    }

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

    QTextEdit* textEdit = new QTextEdit(contentWidget);
    textEdit->setPlaceholderText("Введите текст...");
    textEdit->setMinimumHeight(100);
    textEdit->setMaximumHeight(200);

    QFrame* frame = new QFrame(contentWidget);
    frame->setFrameStyle(QFrame::Box | QFrame::Raised);
    frame->setLineWidth(1);
    QVBoxLayout* frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(5, 5, 5, 5);
    frameLayout->addWidget(textEdit);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    
    QPushButton* upButton = new QPushButton("↑", frame);
    upButton->setMaximumWidth(40);
    QPushButton* downButton = new QPushButton("↓", frame);
    downButton->setMaximumWidth(40);
    QPushButton* removeButton = new QPushButton("Удалить", frame);
    
    buttonsLayout->addWidget(upButton);
    buttonsLayout->addWidget(downButton);
    buttonsLayout->addWidget(removeButton);
    buttonsLayout->addStretch();
    
    frameLayout->addLayout(buttonsLayout);
    
    connect(upButton, &QPushButton::clicked, this, [frame, layout]() {
        int index = layout->indexOf(frame);
        if (index > 0) {
            layout->removeWidget(frame);
            layout->insertWidget(index - 1, frame);
        }
    });
    
    connect(downButton, &QPushButton::clicked, this, [frame, layout]() {
        int index = layout->indexOf(frame);
        if (index >= 0 && index < layout->count() - 1) {
            layout->removeWidget(frame);
            layout->insertWidget(index + 1, frame);
        }
    });
    
    connect(removeButton, &QPushButton::clicked, this, [frame, layout]() {
        layout->removeWidget(frame);
        frame->deleteLater();
    });

    layout->addWidget(frame);
}

void MainWindow::addTableBlockToReport()
{
    QVBoxLayout* layout = ui->reportContentLayout;
    if (!layout) {
        return;
    }
    
    QWidget* contentWidget = ui->reportContentWidget;

    QTableView* tableView = new QTableView(contentWidget);
    tableView->setModel(m_tableModel);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->setMinimumHeight(200);
    tableView->setMaximumHeight(400);

    QFrame* frame = new QFrame(contentWidget);
    frame->setFrameStyle(QFrame::Box | QFrame::Raised);
    frame->setLineWidth(1);
    QVBoxLayout* frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(5, 5, 5, 5);
    frameLayout->addWidget(tableView);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    
    QPushButton* upButton = new QPushButton("↑", frame);
    upButton->setMaximumWidth(40);
    QPushButton* downButton = new QPushButton("↓", frame);
    downButton->setMaximumWidth(40);
    QPushButton* removeButton = new QPushButton("Удалить", frame);
    
    buttonsLayout->addWidget(upButton);
    buttonsLayout->addWidget(downButton);
    buttonsLayout->addWidget(removeButton);
    buttonsLayout->addStretch();
    
    frameLayout->addLayout(buttonsLayout);
    
    connect(upButton, &QPushButton::clicked, this, [frame, layout]() {
        int index = layout->indexOf(frame);
        if (index > 0) {
            layout->removeWidget(frame);
            layout->insertWidget(index - 1, frame);
        }
    });
    
    connect(downButton, &QPushButton::clicked, this, [frame, layout]() {
        int index = layout->indexOf(frame);
        if (index >= 0 && index < layout->count() - 1) {
            layout->removeWidget(frame);
            layout->insertWidget(index + 1, frame);
        }
    });
    
    connect(removeButton, &QPushButton::clicked, this, [frame, layout]() {
        layout->removeWidget(frame);
        frame->deleteLater();
    });

    layout->addWidget(frame);
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

    // Получаем текущий график
    QCustomPlot* sourcePlot = m_plotTabs[currentIndex].plot;

    // Копируем график через pixmap (простой способ)
    QPixmap pixmap = sourcePlot->toPixmap();
    QLabel* plotLabel = new QLabel(contentWidget);
    plotLabel->setPixmap(pixmap);
    plotLabel->setScaledContents(true);
    plotLabel->setMinimumHeight(300);
    plotLabel->setMaximumHeight(500);
    plotLabel->setAlignment(Qt::AlignCenter);

    // Добавляем рамку вокруг блока
    QFrame* frame = new QFrame(contentWidget);
    frame->setFrameStyle(QFrame::Box | QFrame::Raised);
    frame->setLineWidth(1);
    QVBoxLayout* frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(5, 5, 5, 5);
    
    frameLayout->addWidget(plotLabel);

    // Добавляем кнопки управления (Вверх, Вниз, Удалить)
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    
    QPushButton* upButton = new QPushButton("↑", frame);
    upButton->setMaximumWidth(40);
    QPushButton* downButton = new QPushButton("↓", frame);
    downButton->setMaximumWidth(40);
    QPushButton* removeButton = new QPushButton("Удалить", frame);
    
    buttonsLayout->addWidget(upButton);
    buttonsLayout->addWidget(downButton);
    buttonsLayout->addWidget(removeButton);
    buttonsLayout->addStretch();
    
    frameLayout->addLayout(buttonsLayout);
    
    connect(upButton, &QPushButton::clicked, this, [frame, layout]() {
        int index = layout->indexOf(frame);
        if (index > 0) {
            layout->removeWidget(frame);
            layout->insertWidget(index - 1, frame);
        }
    });
    
    connect(downButton, &QPushButton::clicked, this, [frame, layout]() {
        int index = layout->indexOf(frame);
        if (index >= 0 && index < layout->count() - 1) {
            layout->removeWidget(frame);
            layout->insertWidget(index + 1, frame);
        }
    });
    
    connect(removeButton, &QPushButton::clicked, this, [frame, layout]() {
        layout->removeWidget(frame);
        frame->deleteLater();
    });

    layout->addWidget(frame);
}
