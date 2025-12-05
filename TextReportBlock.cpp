#include "TextReportBlock.h"
#include <QTextDocument>
#include <QTextCursor>

TextReportBlock::TextReportBlock(QWidget* parent)
    : ReportBlock(parent)
    , m_textEdit(nullptr)
{
}

QWidget* TextReportBlock::createContent(QWidget* parent)
{
    m_textEdit = new QTextEdit(parent);
    m_textEdit->setPlaceholderText("Введите текст...");
    m_textEdit->setMinimumHeight(100);
    m_textEdit->setMaximumHeight(200);
    return m_textEdit;
}

QString TextReportBlock::getText() const
{
    if (m_textEdit) {
        return m_textEdit->toPlainText();
    }
    return QString();
}

void TextReportBlock::exportToPDF(QTextDocument* document, QTextCursor& cursor)
{
    Q_UNUSED(document);
    QString text = getText();
    if (!text.isEmpty()) {
        cursor.insertText(text);
        cursor.insertBlock();
    }
}

