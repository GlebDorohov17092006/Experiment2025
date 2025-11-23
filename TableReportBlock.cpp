#include "TableReportBlock.h"
#include "TableModel.h"
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextCharFormat>
#include <QTextFrameFormat>
#include <QHeaderView>
#include <QModelIndex>

TableReportBlock::TableReportBlock(TableModel* model, QWidget* parent)
    : ReportBlock(parent)
    , m_tableView(nullptr)
    , m_tableModel(model)
{
}

QWidget* TableReportBlock::createContent(QWidget* parent)
{
    m_tableView = new QTableView(parent);
    m_tableView->setModel(m_tableModel);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableView->setMinimumHeight(200);
    m_tableView->setMaximumHeight(400);
    return m_tableView;
}

void TableReportBlock::exportToPDF(QTextDocument* document, QTextCursor& cursor)
{
    Q_UNUSED(document);
    if (!m_tableModel) {
        return;
    }

    int rowCount = m_tableModel->rowCount();
    int columnCount = m_tableModel->columnCount();

    if (rowCount > 0 && columnCount > 0) {
        // Создаем таблицу в документе
        QTextTableFormat tableFormat;
        tableFormat.setBorder(1);
        tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
        tableFormat.setCellPadding(5);
        tableFormat.setCellSpacing(0);

        QTextTable* table = cursor.insertTable(rowCount + 1, columnCount, tableFormat);

        // Заголовки
        QTextCharFormat headerFormat;
        headerFormat.setFontWeight(QFont::Bold);
        for (int col = 0; col < columnCount; ++col) {
            QString header = m_tableModel->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
            QTextTableCell cell = table->cellAt(0, col);
            QTextCursor cellCursor = cell.firstCursorPosition();
            cellCursor.setCharFormat(headerFormat);
            cellCursor.insertText(header);
        }

        // Данные
        for (int row = 0; row < rowCount; ++row) {
            for (int col = 0; col < columnCount; ++col) {
                QModelIndex index = m_tableModel->index(row, col);
                QString data = m_tableModel->data(index, Qt::DisplayRole).toString();
                QTextTableCell cell = table->cellAt(row + 1, col);
                QTextCursor cellCursor = cell.firstCursorPosition();
                cellCursor.insertText(data);
            }
        }

        cursor.movePosition(QTextCursor::End);
        cursor.insertBlock();
    }
}

