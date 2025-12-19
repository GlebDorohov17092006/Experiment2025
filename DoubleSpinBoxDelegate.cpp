#include "DoubleSpinBoxDelegate.h"
#include <QDoubleSpinBox>
#include <QAbstractItemModel>
#include <QPainter>
#include <QStyleOptionViewItem>

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *DoubleSpinBoxDelegate::createEditor(QWidget *parent,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setFrame(true);
    editor->setMinimum(-1e10);
    editor->setMaximum(1e10);
    editor->setDecimals(3); // Уменьшаем количество знаков после запятой для более чистого отображения
    editor->setSingleStep(0.1);
    editor->setButtonSymbols(QAbstractSpinBox::NoButtons); // Убираем кнопки изменения значения
    
    // Устанавливаем непрозрачный фон с серым цветом и белым текстом
    editor->setAutoFillBackground(true);
    QPalette palette = editor->palette();
    // Серый фон (цвет фона таблицы)
    QColor grayBackground(53, 53, 53); // Темно-серый цвет, соответствующий фону таблицы
    QColor whiteText(255, 255, 255); // Белый текст
    palette.setColor(QPalette::Base, grayBackground);
    palette.setColor(QPalette::Text, whiteText);
    palette.setColor(QPalette::Window, grayBackground);
    palette.setColor(QPalette::WindowText, whiteText);
    palette.setColor(QPalette::Button, grayBackground);
    palette.setColor(QPalette::ButtonText, whiteText);
    editor->setPalette(palette);
    
    // Устанавливаем стиль с серым фоном, белым текстом и полностью убираем кнопки
    editor->setStyleSheet(
        "QDoubleSpinBox { "
        "border: 1px solid #767676; "
        "background-color: #353535; "
        "color: white; "
        "selection-background-color: #3daee9; "
        "selection-color: white; "
        "padding-right: 0px; " // Убираем отступ справа, где обычно кнопки
        "}"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { "
        "width: 0px; "
        "border: none; "
        "}"
    );
    
    // Убеждаемся, что виджет непрозрачный
    editor->setAttribute(Qt::WA_OpaquePaintEvent, true);

    return editor;
}

void DoubleSpinBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(editor);
    if (!spinBox) {
        QStyledItemDelegate::setEditorData(editor, index);
        return;
    }

    // Получаем числовое значение из модели (EditRole всегда возвращает число)
    QVariant value = index.model()->data(index, Qt::EditRole);
    double doubleValue = value.toDouble();
    spinBox->setValue(doubleValue);
    
    // Выделяем весь текст для удобства редактирования
    spinBox->selectAll();
}

void DoubleSpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                         const QModelIndex &index) const
{
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(editor);
    if (!spinBox) {
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    spinBox->interpretText();
    double value = spinBox->value();
    model->setData(index, value, Qt::EditRole);
}

void DoubleSpinBoxDelegate::updateEditorGeometry(QWidget *editor,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &index) const
{
    Q_UNUSED(index);
    // Убеждаемся, что редактор полностью перекрывает ячейку, включая границы
    QRect rect = option.rect;
    // Немного расширяем, чтобы гарантированно перекрыть ячейку
    rect.adjust(-1, -1, 1, 1);
    editor->setGeometry(rect);
    editor->raise(); // Поднимаем редактор поверх всего
}

void DoubleSpinBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    // Используем стандартную отрисовку, но только для DisplayRole
    // Это гарантирует, что при редактировании содержимое ячейки не будет видно
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    
    // Рисуем только если это не редактирование
    if (!(opt.state & QStyle::State_Editing)) {
        QStyledItemDelegate::paint(painter, opt, index);
    }
}

