#ifndef COMBOITEMDELEGATE_H
#define COMBOITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QComboBox>
#include <QStringList>
#include <QVariant>

class ComboItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ComboItemDelegate(QObject *parent = nullptr);
    
    // Добавить элемент в список
    void addItem(const QString &text, const QVariant &data = QVariant());
    void addItems(const QStringList &texts);
    
    QWidget *createEditor(QWidget *parent, 
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override;
    
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                     const QModelIndex &index) const override;
    
    void updateEditorGeometry(QWidget *editor,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const override;

private:
    QStringList m_items;
    QList<QVariant> m_data;
};

#endif // COMBOITEMDELEGATE_H

