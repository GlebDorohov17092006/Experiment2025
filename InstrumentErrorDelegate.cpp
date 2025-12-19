#include "InstrumentErrorDelegate.h"
#include <QDoubleSpinBox>
#include <QAbstractSpinBox>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyle>

InstrumentErrorDelegate::InstrumentErrorDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *InstrumentErrorDelegate::createEditor(QWidget *parent,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setFrame(true);
    editor->setMinimum(0.0);
    editor->setMaximum(1e10);
    editor->setDecimals(3);
    editor->setSingleStep(0.1);
    editor->setButtonSymbols(QAbstractSpinBox::NoButtons);
    
    // Настройка стиля для темной темы
    editor->setStyleSheet(
        "QDoubleSpinBox {"
        "    background-color: #353535;"
        "    color: white;"
        "    border: 1px solid #555555;"
        "    padding: 2px;"
        "}"
        "QDoubleSpinBox:focus {"
        "    border: 1px solid #0078d4;"
        "}"
    );
    
    editor->setAutoFillBackground(true);
    editor->setAttribute(Qt::WA_OpaquePaintEvent, true);
    
    return editor;
}

void InstrumentErrorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(editor);
    if (!spinBox || !index.isValid() || !index.model()) {
        return;
    }

    QVariant data = index.model()->data(index, Qt::EditRole);
    double doubleValue = 0.0;
    
    if (data.type() == QVariant::Double || data.type() == QVariant::Int) {
        doubleValue = data.toDouble();
    } else {
        QString value = data.toString();
        // Убираем символ процента, если он есть
        value = value.replace("%", "").trimmed();
        
        bool ok;
        doubleValue = value.toDouble(&ok);
        if (!ok) {
            doubleValue = 0.0;
        }
    }
    
    spinBox->setValue(doubleValue);
    spinBox->selectAll();
}

void InstrumentErrorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                            const QModelIndex &index) const
{
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(editor);
    if (!spinBox || !model || !index.isValid()) {
        return;
    }

    double value = spinBox->value();
    // Сохраняем как число, форматирование с % будет сделано в модели
    model->setData(index, value, Qt::EditRole);
}

void InstrumentErrorDelegate::updateEditorGeometry(QWidget *editor,
                                                  const QStyleOptionViewItem &option,
                                                  const QModelIndex &index) const
{
    Q_UNUSED(index);
    QRect rect = option.rect;
    rect.adjust(-1, -1, 1, 1);
    editor->setGeometry(rect);
    editor->raise();
}

void InstrumentErrorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    if (!(opt.state & QStyle::State_Editing)) {
        QStyledItemDelegate::paint(painter, opt, index);
    }
}

