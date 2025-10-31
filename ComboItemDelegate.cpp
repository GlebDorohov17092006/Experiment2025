#include "ComboItemDelegate.h"
#include <QComboBox>

ComboItemDelegate::ComboItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ComboItemDelegate::addItem(const QString &text, const QVariant &data)
{
    m_items.append(text);
    m_data.append(data.isValid() ? data : QVariant(text));
}

void ComboItemDelegate::addItems(const QStringList &texts)
{
    m_items.append(texts);
    for (const QString &text : texts) {
        m_data.append(QVariant(text));
    }
}

QWidget *ComboItemDelegate::createEditor(QWidget *parent,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    
    QComboBox *editor = new QComboBox(parent);
    
    if (!m_data.isEmpty() && m_data.size() == m_items.size()) {
        for (int i = 0; i < m_items.size(); ++i) {
            editor->addItem(m_items[i], m_data[i]);
        }
    } else {
        editor->addItems(m_items);
    }
    
    return editor;
}

void ComboItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    if (!comboBox) {
        QStyledItemDelegate::setEditorData(editor, index);
        return;
    }
    
    QString currentText = index.model()->data(index, Qt::EditRole).toString();
    int idx = comboBox->findText(currentText);
    
    if (idx < 0 && !m_data.isEmpty()) {
        QVariant currentData = index.model()->data(index, Qt::EditRole);
        idx = comboBox->findData(currentData);
    }
    
    if (idx >= 0) {
        comboBox->setCurrentIndex(idx);
    }
}

void ComboItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    if (!comboBox) {
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }
    
    QString selectedText = comboBox->currentText();
    model->setData(index, selectedText, Qt::EditRole);
}

void ComboItemDelegate::updateEditorGeometry(QWidget *editor,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

