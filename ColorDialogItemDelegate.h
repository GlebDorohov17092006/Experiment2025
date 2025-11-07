#ifndef COLORDIALOGITEMDELEGATE_H
#define COLORDIALOGITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QColorDialog>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>

class ColorDialogItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ColorDialogItemDelegate(QObject *parent = nullptr);
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                    const QStyleOptionViewItem &option,
                    const QModelIndex &index) override;

private:
    QColor getColorFromIndex(const QModelIndex &index) const;
};

#endif // COLORDIALOGITEMDELEGATE_H

