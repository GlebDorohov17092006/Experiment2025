#ifndef REPORTBLOCK_H
#define REPORTBLOCK_H

#include <QFrame>
#include <QWidget>
#include <QVBoxLayout>
#include <QObject>

class QTextDocument;
class QTextCursor;

class ReportBlock : public QObject
{
    Q_OBJECT

public:
    ReportBlock(QWidget* parent = nullptr);
    virtual ~ReportBlock() = default;

    QFrame* createWidget(QWidget* parent, QVBoxLayout* parentLayout);

    virtual void exportToPDF(QTextDocument* document, QTextCursor& cursor) = 0;

    QFrame* getFrame() const { return m_frame; }

protected:
    virtual QWidget* createContent(QWidget* parent) = 0;

    void createControlButtons(QFrame* frame, QVBoxLayout* frameLayout, QVBoxLayout* parentLayout);

    QFrame* m_frame;
    QVBoxLayout* m_parentLayout;
};

#endif // REPORTBLOCK_H

