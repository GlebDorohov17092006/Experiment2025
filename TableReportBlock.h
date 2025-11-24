#ifndef TABLEREPORTBLOCK_H
#define TABLEREPORTBLOCK_H

#include "ReportBlock.h"
#include <QTableView>

class TableModel;

class TableReportBlock : public ReportBlock
{
public:
    TableReportBlock(TableModel* model, QWidget* parent = nullptr);
    ~TableReportBlock() override = default;

    void exportToPDF(QTextDocument* document, QTextCursor& cursor) override;

protected:
    QWidget* createContent(QWidget* parent) override;

private:
    QTableView* m_tableView;
    TableModel* m_tableModel;
};

#endif // TABLEREPORTBLOCK_H

