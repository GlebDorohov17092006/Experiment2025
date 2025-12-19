#include "LineEditDelegate.h"
#include <QLineEdit>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyle>

LineEditDelegate::LineEditDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *LineEditDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QLineEdit *editor = new QLineEdit(parent);
    
    // Настройка стиля для темной темы
    editor->setStyleSheet(
        "QLineEdit {"
        "    background-color: #353535;"
        "    color: white;"
        "    border: 1px solid #555555;"
        "    padding: 2px;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid #0078d4;"
        "}"
    );
    
    editor->setAutoFillBackground(true);
    editor->setAttribute(Qt::WA_OpaquePaintEvent, true);
    
    return editor;
}

void LineEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    if (!lineEdit) {
        return;
    }

    QString value = index.model()->data(index, Qt::EditRole).toString();
    lineEdit->setText(value);
    lineEdit->selectAll();
}

void LineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    if (!lineEdit) {
        return;
    }

    QString value = lineEdit->text();
    model->setData(index, value, Qt::EditRole);
}

void LineEditDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    Q_UNUSED(index);
    QRect rect = option.rect;
    rect.adjust(-1, -1, 1, 1);
    editor->setGeometry(rect);
    editor->raise();
}

void LineEditDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    if (!(opt.state & QStyle::State_Editing)) {
        QStyledItemDelegate::paint(painter, opt, index);
    }
}

