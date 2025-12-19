#include "TableModel.h"
#include "Variable.h"
#include "Instrument.h"
#include <QModelIndex>
#include <QDebug>

TableModel::TableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    auto experiment = Experiment::get_instance();
    if (!experiment)
        return 0;

    size_t maxRows = 0;
    for (size_t i = 0; i < experiment->get_variables_count(); ++i) {
        auto& var = experiment->get_variable(i);
        maxRows = std::max(maxRows, var.get_measurements_count());
    }
    return static_cast<int>(maxRows);
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    auto experiment = Experiment::get_instance();
    if (!experiment)
        return 0;

    return static_cast<int>(experiment->get_variables_count());
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto experiment = Experiment::get_instance();
    if (!experiment)
        return QVariant();

    int col = index.column();
    int row = index.row();

    if (col >= static_cast<int>(experiment->get_variables_count()))
        return QVariant();

    auto& variable = experiment->get_variable(col);

    if (role == Qt::EditRole) {
        // Для редактирования всегда возвращаем только числовое значение
        if (row < static_cast<int>(variable.get_measurements_count())) {
            return variable.get_measurement(row);
        } else {
            return 0.0;
        }
    }
    
    if (role == Qt::DisplayRole) {
        // Для отображения показываем значение с погрешностью
        if (row < static_cast<int>(variable.get_measurements_count())) {
            double value = variable.get_measurement(row);
            double error = variable.get_error_instrument(0, value);
            if (error > 0) {
                return QString("%1 ± %2").arg(value, 0, 'f', 3).arg(error, 0, 'f', 3);
            } else {
                return QString::number(value, 'f', 3);
            }
        } else {
            return "0.000";
        }
    }

    return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    auto experiment = Experiment::get_instance();
    if (!experiment)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if (section >= static_cast<int>(experiment->get_variables_count()))
            return QString("Переменная %1").arg(section + 1);

        auto& variable = experiment->get_variable(section);
        QString name = QString::fromStdString(variable.get_name_tables());
        QString tag = QString::fromStdString(variable.get_name_calculated());

        if (!tag.isEmpty()) {
            return QString("%1\n(%2)").arg(name).arg(tag);
        }
        return name;
    } else {
        return section + 1;
    }
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    qDebug() << "setData called:" << index.row() << index.column() << value;

    auto experiment = Experiment::get_instance();
    if (!experiment)
        return false;

    int col = index.column();
    int row = index.row();

    if (col >= static_cast<int>(experiment->get_variables_count()))
        return false;

    auto& variable = experiment->get_variable(col);

    bool ok;
    double doubleValue = value.toDouble(&ok);

    if (!ok) {
        qDebug() << "Failed to convert to double";
        return false;
    }

    // Убеждаемся, что есть достаточно измерений
    while (static_cast<int>(variable.get_measurements_count()) <= row) {
        qDebug() << "Adding measurement at row" << row;
        variable.add_measurement(0.0);
    }

    qDebug() << "Setting measurement:" << row << "=" << doubleValue;
    variable.set_measurement(row, doubleValue);

    emit dataChanged(index, index, {role});

    return true;
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);

    if (index.isValid()) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

void TableModel::refreshData()
{
    beginResetModel();

    // Принудительно обновляем данные
    auto experiment = Experiment::get_instance();
    if (experiment) {
        qDebug() << "TableModel refreshData: variables count =" << experiment->get_variables_count();
        if (experiment->get_variables_count() > 0) {
            qDebug() << "First variable measurements count =" << experiment->get_variable(0).get_measurements_count();
        }
    }

    endResetModel();

    // Испускаем сигналы об изменении данных
    emit dataChanged(createIndex(0, 0),
                    createIndex(rowCount() - 1, columnCount() - 1));
    emit layoutChanged();
}

void TableModel::refreshColumn(int column)
{
    if (column < 0 || column >= columnCount())
        return;

    int rows = rowCount();
    if (rows > 0) {
        QModelIndex topLeft = createIndex(0, column);
        QModelIndex bottomRight = createIndex(rows - 1, column);
        emit dataChanged(topLeft, bottomRight);
    }
}
