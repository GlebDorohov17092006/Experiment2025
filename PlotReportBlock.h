#ifndef PLOTREPORTBLOCK_H
#define PLOTREPORTBLOCK_H

#include "ReportBlock.h"
#include <QLabel>
#include <QPixmap>

class PlotReportBlock : public ReportBlock
{
public:
    PlotReportBlock(const QPixmap& pixmap, QWidget* parent = nullptr);
    ~PlotReportBlock() override = default;

    void exportToPDF(QTextDocument* document, QTextCursor& cursor) override;

protected:
    QWidget* createContent(QWidget* parent) override;

private:
    QLabel* m_plotLabel;
    QPixmap m_pixmap;
};

#endif // PLOTREPORTBLOCK_H

