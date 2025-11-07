#include "ColorDialogItemDelegate.h"
#include <QColorDialog>
#include <QApplication>
#include <QStyleOptionViewItem>
#include <QAbstractItemModel>
#include <QMouseEvent>
#include <QEvent>

ColorDialogItemDelegate::ColorDialogItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ColorDialogItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QColor color = getColorFromIndex(index);
    
    QRect rect = option.rect;
    rect.adjust(10, 10, -5, -5);
    
    painter->save();
    
    painter->setBrush(QBrush(color));
    painter->drawRect(rect);
    
    painter->restore();
}

bool ColorDialogItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton && option.rect.contains(mouseEvent->pos())) {
            QColor currentColor = getColorFromIndex(index);
            
            QColorDialog dialog;
            dialog.setCurrentColor(currentColor);
            dialog.setOption(QColorDialog::ShowAlphaChannel, false);
            
            if (dialog.exec() == QDialog::Accepted) {
                QColor selectedColor = dialog.selectedColor();
                model->setData(index, selectedColor.name(QColor::HexRgb), Qt::EditRole);
                return true;
            }
        }
    }
    
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QColor ColorDialogItemDelegate::getColorFromIndex(const QModelIndex &index) const
{
    QVariant data = index.model()->data(index, Qt::EditRole);
    
    if (data.type() == QVariant::Color) {
        return data.value<QColor>();
    } else if (data.type() == QVariant::String) {
        QString colorString = data.toString();
        QColor color(colorString);
        if (color.isValid()) {
            return color;
        }
    }
    
    // Возвращаем черный цвет по умолчанию, если данные невалидны
    return QColor(Qt::black);
}

