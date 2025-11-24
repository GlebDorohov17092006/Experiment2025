#ifndef TEXTREPORTBLOCK_H
#define TEXTREPORTBLOCK_H

#include "ReportBlock.h"
#include <QTextEdit>

class TextReportBlock : public ReportBlock
{
public:
    TextReportBlock(QWidget* parent = nullptr);
    ~TextReportBlock() override = default;

    void exportToPDF(QTextDocument* document, QTextCursor& cursor) override;

    QString getText() const;

protected:
    QWidget* createContent(QWidget* parent) override;

private:
    QTextEdit* m_textEdit;
};

#endif // TEXTREPORTBLOCK_H

