#include "OpacitySpinBoxDelegate.h"
#include <QSpinBox>
#include <QAbstractSpinBox>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyle>

OpacitySpinBoxDelegate::OpacitySpinBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *OpacitySpinBoxDelegate::createEditor(QWidget *parent,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QSpinBox *editor = new QSpinBox(parent);
    editor->setRange(0, 100);
    editor->setSuffix(" %");
    editor->setButtonSymbols(QAbstractSpinBox::NoButtons);
    editor->setAutoFillBackground(true);
    
    // Настройка стиля для темной темы
    editor->setStyleSheet(
        "QSpinBox {"
        "    background-color: #353535;"
        "    color: white;"
        "    border: 1px solid #555555;"
        "    padding: 2px;"
        "}"
        "QSpinBox:focus {"
        "    border: 1px solid #0078d4;"
        "}"
    );
    
    editor->setAttribute(Qt::WA_OpaquePaintEvent, true);
    
    return editor;
}

void OpacitySpinBoxDelegate::setEditorData(QWidget *editor,
                                            const QModelIndex &index) const
{
    QSpinBox *spinBox = qobject_cast<QSpinBox*>(editor);
    if (!spinBox) {
        return;
    }

    QString value = index.model()->data(index, Qt::EditRole).toString();
    // Убираем символ процента, если он есть
    value = value.replace(" %", "").trimmed();
    
    bool ok;
    int intValue = value.toInt(&ok);
    if (ok) {
        spinBox->setValue(intValue);
    } else {
        spinBox->setValue(100); // По умолчанию 100%
    }
    
    spinBox->selectAll();
}

void OpacitySpinBoxDelegate::setModelData(QWidget *editor,
                                           QAbstractItemModel *model,
                                           const QModelIndex &index) const
{
    QSpinBox *spinBox = qobject_cast<QSpinBox*>(editor);
    if (!spinBox) {
        return;
    }

    int value = spinBox->value();
    model->setData(index, QString::number(value) + " %", Qt::EditRole);
    model->setData(index, QString::number(value) + " %", Qt::DisplayRole);
}

void OpacitySpinBoxDelegate::updateEditorGeometry(QWidget *editor,
                                                  const QStyleOptionViewItem &option,
                                                  const QModelIndex &index) const
{
    Q_UNUSED(index);
    QRect rect = option.rect;
    rect.adjust(-1, -1, 1, 1);
    editor->setGeometry(rect);
    editor->raise();
}

void OpacitySpinBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    if (!(opt.state & QStyle::State_Editing)) {
        QStyledItemDelegate::paint(painter, opt, index);
    }
}

