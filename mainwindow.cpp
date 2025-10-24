#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "reportdialog.h"
#include "qcustomplot.h"
#include <QInputDialog>
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
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->linePlot->setInteraction(QCP::iRangeDrag, true);
    ui->linePlot->setInteraction(QCP::iRangeZoom, true);

    connect(ui->tableWidget->horizontalHeader(), &QHeaderView::sectionDoubleClicked,
            this, &MainWindow::onColumnHeaderDoubleClicked);
    
    ui->instrumentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->instrumentsTable->setColumnWidth(0, 150);
    ui->instrumentsTable->setColumnWidth(1, 300);
    ui->instrumentsTable->setColumnWidth(2, 100);
    
    connect(ui->addColumnButton, &QPushButton::clicked, this, &MainWindow::addColumn);
    connect(ui->removeColumnButton, &QPushButton::clicked, this, &MainWindow::removeColumn);
    connect(ui->addRowButton, &QPushButton::clicked, this, &MainWindow::addRow);
    connect(ui->removeRowButton, &QPushButton::clicked, this, &MainWindow::removeRow);
    
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::addInstrument);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::removeInstrument);
    
    // Автоматическая перерисовка графика при изменении данных в таблице
    connect(ui->tableWidget, &QTableWidget::cellChanged, this, &MainWindow::drawSimpleGraph);
    
    // Подключаем пункт меню "Создать отчет"
    connect(ui->action_3, &QAction::triggered, this, &MainWindow::openReportDialog);
    
    // Синхронизация вкладок tabPlotSettings и tabPlot
    connect(ui->tabPlotSettings, &QTabWidget::currentChanged, this, &MainWindow::onPlotSettingsTabChanged);
    connect(ui->tabPlot, &QTabWidget::currentChanged, this, &MainWindow::onPlotTabChanged);
    
    // Подключаем обновление названий осей при изменении полей
    connect(ui->lineEdit_xAxisLabel, &QLineEdit::textChanged, this, &MainWindow::drawSimpleGraph);
    connect(ui->lineEdit_yAxisLabel, &QLineEdit::textChanged, this, &MainWindow::drawSimpleGraph);
    
    // Обработчик двойного клика для выбора инструмента
    connect(ui->variableInstrumentsTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::onInstrumentCellDoubleClicked);
    
    // Обновляем тексты инструментов при изменении таблицы инструментов
    connect(ui->instrumentsTable, &QTableWidget::cellChanged, this, &MainWindow::updateInstrumentTexts);
    
    // Настройка таблицы инструментов для переменных
    ui->variableInstrumentsTable->horizontalHeader()->setDefaultSectionSize(200);
    
    // Синхронизируем таблицу инструментов
    syncVariableInstrumentsTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::drawSimpleGraph()
{
    ui->linePlot->clearGraphs();
    
    int columnCount = ui->tableWidget->columnCount();
    int rowCount = ui->tableWidget->rowCount();
    
    if (columnCount == 0 || rowCount == 0) {
        return;
    }
    

    QVector<QColor> colors = {
        Qt::blue, Qt::red, Qt::green, Qt::magenta, 
        Qt::cyan, Qt::darkYellow, Qt::darkBlue, Qt::darkRed,
        Qt::darkGreen, Qt::darkMagenta, Qt::darkCyan
    };
    
    for (int col = 0; col < columnCount; ++col) {
        QVector<double> x, y;
        
        for (int row = 0; row < rowCount; ++row) {
            QTableWidgetItem* item = ui->tableWidget->item(row, col);
            
            if (item && !item->text().isEmpty()) {
                bool ok;
                double value = item->text().toDouble(&ok);
                
                if (ok) {
                    x.append(row + 1);
                    y.append(value);
                }
            }
        }
        
        if (!x.isEmpty() && !y.isEmpty()) {
            ui->linePlot->addGraph();
            int graphIndex = ui->linePlot->graphCount() - 1;
            
            ui->linePlot->graph(graphIndex)->setData(x, y);
            
            QColor color = colors[col % colors.size()];
            ui->linePlot->graph(graphIndex)->setPen(QPen(color, 2));
            ui->linePlot->graph(graphIndex)->setLineStyle(QCPGraph::lsLine);
            ui->linePlot->graph(graphIndex)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
            
            QString columnName = getColumnName(col);
            ui->linePlot->graph(graphIndex)->setName(columnName);
        }
    }
    
    if (ui->linePlot->graphCount() == 0) {
        return;
    }
    
    QString xLabel = ui->lineEdit_xAxisLabel->text().trimmed();
    QString yLabel = ui->lineEdit_yAxisLabel->text().trimmed();
    

    
    // Настройка осей
    ui->linePlot->xAxis->setLabel(xLabel);
    ui->linePlot->yAxis->setLabel(yLabel);
    
    // Автоматическое масштабирование осей по данным
    ui->linePlot->rescaleAxes();
    
    // Обновляем отображение
    ui->linePlot->replot();
}

void MainWindow::onPlotSettingsTabChanged(int index)
{
    // Синхронизируем табы: когда меняется вкладка настроек, меняем вкладку графика
    // Временно отключаем сигнал, чтобы избежать рекурсии
    ui->tabPlot->blockSignals(true);
    ui->tabPlot->setCurrentIndex(index);
    ui->tabPlot->blockSignals(false);
}

void MainWindow::onPlotTabChanged(int index)
{
    // Синхронизируем табы: когда меняется вкладка графика, меняем вкладку настроек
    // Временно отключаем сигнал, чтобы избежать рекурсии
    ui->tabPlotSettings->blockSignals(true);
    ui->tabPlotSettings->setCurrentIndex(index);
    ui->tabPlotSettings->blockSignals(false);
}

void MainWindow::addColumn()
{
    int currentColumns = ui->tableWidget->columnCount();
    
    ui->tableWidget->insertColumn(currentColumns);
    
    QString headerName = QString("Столбец %1").arg(currentColumns + 1);
    ui->tableWidget->setHorizontalHeaderItem(currentColumns, new QTableWidgetItem(headerName));
    
    // Добавляем столбец в таблицу инструментов для переменных
    ui->variableInstrumentsTable->insertColumn(currentColumns);
    ui->variableInstrumentsTable->setHorizontalHeaderItem(currentColumns, new QTableWidgetItem(headerName));
    
    // Создаем ячейку с текстом вместо ComboBox
    QTableWidgetItem* item = new QTableWidgetItem("(не выбран)");
    item->setData(Qt::UserRole, -1); // -1 означает, что инструмент не выбран
    ui->variableInstrumentsTable->setItem(0, currentColumns, item);
    
    // Добавляем строку в таблицу настроек графика
    addPlotSettingsRow(headerName);
    
    // Перерисовываем график
    drawSimpleGraph();
}

void MainWindow::removeColumn()
{
    QItemSelectionModel* selectionModel = ui->tableWidget->selectionModel();
    if (selectionModel && selectionModel->hasSelection()) {
        QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
        if (!selectedIndexes.isEmpty()) {
            QSet<int> fullySelectedColumns;
            int totalRows = ui->tableWidget->rowCount();
            
            QMap<int, QSet<int>> columnRows;
            for (const QModelIndex& index : selectedIndexes) {
                columnRows[index.column()].insert(index.row());
            }
            
            for (auto it = columnRows.begin(); it != columnRows.end(); ++it) {
                int col = it.key();
                const QSet<int>& rows = it.value();
                
                if (rows.size() == totalRows) {
                    fullySelectedColumns.insert(col);
                }
            }
            
            if (!fullySelectedColumns.isEmpty()) {
                QList<int> sortedColumns = fullySelectedColumns.values();
                std::sort(sortedColumns.begin(), sortedColumns.end(), std::greater<int>());
                
                for (int col : sortedColumns) {
                    ui->tableWidget->removeColumn(col);
                    
                    // Удаляем столбец из таблицы инструментов для переменных
                    if (col < ui->variableInstrumentsTable->columnCount()) {
                        ui->variableInstrumentsTable->removeColumn(col);
                    }
                    
                    // Удаляем соответствующую строку из таблиц настроек графиков
                    if (col < ui->tableWidget_2->rowCount()) {
                        ui->tableWidget_2->removeRow(col);
                    }
                    if (col < ui->tableWidget_5->rowCount()) {
                        ui->tableWidget_5->removeRow(col);
                    }
                    if (col < ui->tableWidget_4->rowCount()) {
                        ui->tableWidget_4->removeRow(col);
                    }
                }
                
                // Перерисовываем график
                drawSimpleGraph();
                return;
            }
        }
    }
}

void MainWindow::addRow()
{
    int currentRows = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(currentRows);
}

void MainWindow::removeRow()
{
    QItemSelectionModel* selectionModel = ui->tableWidget->selectionModel();
    if (selectionModel && selectionModel->hasSelection()) {
        QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
        if (!selectedIndexes.isEmpty()) {
            QSet<int> fullySelectedRows;
            int totalColumns = ui->tableWidget->columnCount();
            
            QMap<int, QSet<int>> rowColumns;
            for (const QModelIndex& index : selectedIndexes) {
                rowColumns[index.row()].insert(index.column());
            }
            
            for (auto it = rowColumns.begin(); it != rowColumns.end(); ++it) {
                int row = it.key();
                const QSet<int>& columns = it.value();
                
                if (columns.size() == totalColumns) {
                    fullySelectedRows.insert(row);
                }
            }
            
            if (!fullySelectedRows.isEmpty()) {
                QList<int> sortedRows = fullySelectedRows.values();
                std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
                
                for (int row : sortedRows) {
                    ui->tableWidget->removeRow(row);
                }
                
                // Перерисовываем график
                drawSimpleGraph();
                return;
            }
        }
    }
}

void MainWindow::addInstrument()
{
    int currentRows = ui->instrumentsTable->rowCount();
    ui->instrumentsTable->insertRow(currentRows);
}

void MainWindow::removeInstrument()
{
    QItemSelectionModel* selectionModel = ui->instrumentsTable->selectionModel();
    if (selectionModel && selectionModel->hasSelection()) {
        QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
        if (!selectedIndexes.isEmpty()) {
            QSet<int> fullySelectedRows;
            int totalColumns = ui->instrumentsTable->columnCount();
            
            QMap<int, QSet<int>> rowColumns;
            for (const QModelIndex& index : selectedIndexes) {
                rowColumns[index.row()].insert(index.column());
            }
            
            for (auto it = rowColumns.begin(); it != rowColumns.end(); ++it) {
                int row = it.key();
                const QSet<int>& columns = it.value();
                
                if (columns.size() == totalColumns) {
                    fullySelectedRows.insert(row);
                }
            }
            
            if (!fullySelectedRows.isEmpty()) {
                QList<int> sortedRows = fullySelectedRows.values();
                std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
                
                for (int row : sortedRows) {
                    ui->instrumentsTable->removeRow(row);
                }
                
                // Обновляем тексты инструментов
                updateInstrumentTexts();
            }
        }
    }
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
            
            ui->tableWidget->setHorizontalHeaderItem(col, new QTableWidgetItem(headerText));
            
            // Обновляем вертикальный заголовок в таблицах настроек графиков
            if (col < ui->tableWidget_2->rowCount()) {
                ui->tableWidget_2->setVerticalHeaderItem(col, new QTableWidgetItem(newName));
            }
            if (col < ui->tableWidget_5->rowCount()) {
                ui->tableWidget_5->setVerticalHeaderItem(col, new QTableWidgetItem(newName));
            }
            if (col < ui->tableWidget_4->rowCount()) {
                ui->tableWidget_4->setVerticalHeaderItem(col, new QTableWidgetItem(newName));
            }
            
            // Перерисовываем график, так как изменились подписи осей
            drawSimpleGraph();
        }
    }
}

QString MainWindow::getColumnName(int columnIndex)
{
    QTableWidgetItem* headerItem = ui->tableWidget->horizontalHeaderItem(columnIndex);
    if (headerItem) {
        QString fullText = headerItem->text();
        int newlinePos = fullText.indexOf('\n');
        if (newlinePos != -1) {
            return fullText.left(newlinePos);
        }
        return fullText;
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

void MainWindow::openReportDialog()
{
    ReportDialog dialog(this);
    dialog.exec();
}

void MainWindow::syncVariableInstrumentsTable()
{
    int mainTableColumns = ui->tableWidget->columnCount();
    int instrumentTableColumns = ui->variableInstrumentsTable->columnCount();
    
    // Синхронизируем количество столбцов
    while (instrumentTableColumns < mainTableColumns) {
        ui->variableInstrumentsTable->insertColumn(instrumentTableColumns);
        QString columnName = getColumnName(instrumentTableColumns);
        ui->variableInstrumentsTable->setHorizontalHeaderItem(instrumentTableColumns, new QTableWidgetItem(columnName));
        
        // Создаем ячейку с текстом вместо ComboBox
        QTableWidgetItem* item = new QTableWidgetItem("(не выбран)");
        item->setData(Qt::UserRole, -1); // -1 означает, что инструмент не выбран
        ui->variableInstrumentsTable->setItem(0, instrumentTableColumns, item);
        
        instrumentTableColumns++;
    }
    
    // Обновляем заголовки столбцов
    for (int i = 0; i < mainTableColumns && i < ui->variableInstrumentsTable->columnCount(); ++i) {
        QString columnName = getColumnName(i);
        ui->variableInstrumentsTable->setHorizontalHeaderItem(i, new QTableWidgetItem(columnName));
    }
    
    // Синхронизируем таблицы настроек графиков
    syncPlotSettingsTables();
}

void MainWindow::syncPlotSettingsTables()
{
    int mainTableColumns = ui->tableWidget->columnCount();
    
    // Синхронизируем таблицы настроек графиков
    while (ui->tableWidget_2->rowCount() < mainTableColumns) {
        QString columnName = getColumnName(ui->tableWidget_2->rowCount());
        addPlotSettingsRow(columnName);
    }
    
    // Обновляем вертикальные заголовки
    for (int i = 0; i < mainTableColumns; ++i) {
        QString columnName = getColumnName(i);
        if (i < ui->tableWidget_2->rowCount()) {
            ui->tableWidget_2->setVerticalHeaderItem(i, new QTableWidgetItem(columnName));
        }
        if (i < ui->tableWidget_5->rowCount()) {
            ui->tableWidget_5->setVerticalHeaderItem(i, new QTableWidgetItem(columnName));
        }
        if (i < ui->tableWidget_4->rowCount()) {
            ui->tableWidget_4->setVerticalHeaderItem(i, new QTableWidgetItem(columnName));
        }
    }
}

QString MainWindow::getInstrumentDisplayText(int instrumentIndex)
{
    if (instrumentIndex < 0 || instrumentIndex >= ui->instrumentsTable->rowCount()) {
        return "(не выбран)";
    }
    
    QTableWidgetItem* nameItem = ui->instrumentsTable->item(instrumentIndex, 0);
    QTableWidgetItem* errorItem = ui->instrumentsTable->item(instrumentIndex, 2);
    
    if (!nameItem || nameItem->text().isEmpty()) {
        return "(не выбран)";
    }
    
    QString displayText = nameItem->text();
    if (errorItem && !errorItem->text().isEmpty()) {
        displayText += QString(" (±%1)").arg(errorItem->text());
    }
    
    return displayText;
}

void MainWindow::updateInstrumentTexts()
{
    // Обновляем текст во всех ячейках таблицы инструментов
    int columnCount = ui->variableInstrumentsTable->columnCount();
    
    for (int col = 0; col < columnCount; ++col) {
        QTableWidgetItem* item = ui->variableInstrumentsTable->item(0, col);
        if (item) {
            int instrumentIndex = item->data(Qt::UserRole).toInt();
            QString displayText = getInstrumentDisplayText(instrumentIndex);
            item->setText(displayText);
        }
    }
}

void MainWindow::onInstrumentCellDoubleClicked(int row, int column)
{
    if (row != 0) return; // У нас только одна строка
    
    QTableWidgetItem* item = ui->variableInstrumentsTable->item(row, column);
    if (!item) return;
    
    // Сохраняем текущий выбор
    int currentInstrument = item->data(Qt::UserRole).toInt();
    
    // Создаем временный ComboBox
    QComboBox* comboBox = new QComboBox(ui->variableInstrumentsTable);
    comboBox->addItem("(не выбран)", -1);
    
    // Заполняем список инструментами
    int instrumentCount = ui->instrumentsTable->rowCount();
    for (int i = 0; i < instrumentCount; ++i) {
        QString displayText = getInstrumentDisplayText(i);
        if (displayText != "(не выбран)") {
            comboBox->addItem(displayText, i);
        }
    }
    
    // Устанавливаем текущий выбор
    int comboIndex = comboBox->findData(currentInstrument);
    if (comboIndex >= 0) {
        comboBox->setCurrentIndex(comboIndex);
    }
    
    // Показываем ComboBox
    ui->variableInstrumentsTable->setCellWidget(row, column, comboBox);
    comboBox->showPopup();
    
    // Обработчик выбора значения
    connect(comboBox, QOverload<int>::of(&QComboBox::activated), this, 
            [this, row, column, comboBox]() {
                int selectedInstrument = comboBox->currentData().toInt();
                
                // Создаем или обновляем ячейку
                QTableWidgetItem* item = ui->variableInstrumentsTable->item(row, column);
                if (!item) {
                    item = new QTableWidgetItem();
                    ui->variableInstrumentsTable->setItem(row, column, item);
                }
                
                // Сохраняем выбранный инструмент
                item->setData(Qt::UserRole, selectedInstrument);
                
                // Обновляем текст
                QString displayText = getInstrumentDisplayText(selectedInstrument);
                item->setText(displayText);
                
                // Удаляем ComboBox
                ui->variableInstrumentsTable->removeCellWidget(row, column);
                comboBox->deleteLater();
            });
    
    // Обработчик закрытия без выбора
    connect(comboBox, &QComboBox::destroyed, this, [this, row, column]() {
        ui->variableInstrumentsTable->removeCellWidget(row, column);
    });
}

void MainWindow::addPlotSettingsRow(const QString& columnName)
{
    int rowIndex = ui->tableWidget_2->rowCount();
    ui->tableWidget_2->insertRow(rowIndex);
    
    ui->tableWidget_2->setVerticalHeaderItem(rowIndex, new QTableWidgetItem(columnName));
    
    QTableWidgetItem* checkItem = new QTableWidgetItem();
    checkItem->setCheckState(Qt::Checked);
    ui->tableWidget_2->setItem(rowIndex, 0, checkItem);
    
    ui->tableWidget_2->setItem(rowIndex, 1, new QTableWidgetItem(""));
    ui->tableWidget_2->setItem(rowIndex, 2, new QTableWidgetItem(""));
    ui->tableWidget_2->setItem(rowIndex, 3, new QTableWidgetItem(""));
    ui->tableWidget_2->setItem(rowIndex, 4, new QTableWidgetItem(""));
    ui->tableWidget_2->setItem(rowIndex, 5, new QTableWidgetItem(""));
    
    rowIndex = ui->tableWidget_5->rowCount();
    ui->tableWidget_5->insertRow(rowIndex);
    ui->tableWidget_5->setVerticalHeaderItem(rowIndex, new QTableWidgetItem(columnName));
    
    QTableWidgetItem* checkItem2 = new QTableWidgetItem();
    checkItem2->setCheckState(Qt::Checked);
    ui->tableWidget_5->setItem(rowIndex, 0, checkItem2);
    
    ui->tableWidget_5->setItem(rowIndex, 1, new QTableWidgetItem(""));
    ui->tableWidget_5->setItem(rowIndex, 2, new QTableWidgetItem(""));
    
    rowIndex = ui->tableWidget_4->rowCount();
    ui->tableWidget_4->insertRow(rowIndex);
    ui->tableWidget_4->setVerticalHeaderItem(rowIndex, new QTableWidgetItem(columnName));
    
    QTableWidgetItem* checkItem3 = new QTableWidgetItem();
    checkItem3->setCheckState(Qt::Checked);
    ui->tableWidget_4->setItem(rowIndex, 0, checkItem3);
    
    ui->tableWidget_4->setItem(rowIndex, 1, new QTableWidgetItem(""));
    ui->tableWidget_4->setItem(rowIndex, 2, new QTableWidgetItem(""));
}
