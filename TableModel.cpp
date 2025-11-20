#include "TableModel.h"
#include "Variable.h"
#include "Instrument.h"

TableModel::TableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_experiment(Experiment::get_instance())
{
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    size_t maxRows = 0;
    for (size_t i = 0; i < m_experiment->get_variables_count(); ++i) {
        auto var = m_experiment->get_variable(i);
        maxRows = std::max(maxRows, var->get_measurements_count());
    }
    return static_cast<int>(maxRows);
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(m_experiment->get_variables_count());
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int col = index.column();
    int row = index.row();

    auto variable = m_experiment->get_variable(col);

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (row < static_cast<int>(variable->get_measurements_count())) {
            double value = variable->get_measurement(row);

            if (role == Qt::DisplayRole) {
                double error = variable->get_error_instrument(0, value);
                if (error > 0) {
                    return QString("%1 ± %2").arg(value, 0, 'f', 3).arg(error, 0, 'f', 3);
                }
                return QString::number(value, 'f', 3);
            } else {
                return value;
            }
        }
    }

    return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        auto variable = m_experiment->get_variable(section);
        QString name = QString::fromStdString(variable->get_name_tables());
        QString tag = QString::fromStdString(variable->get_name_calculated());

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

    int col = index.column();
    int row = index.row();

    auto variable = m_experiment->get_variable(col);

    bool ok;
    double doubleValue = value.toDouble(&ok);

    if (!ok)
        return false;

    if (row < static_cast<int>(variable->get_measurements_count())) {
        variable->set_measurement(row, doubleValue);
    } else {
        while (static_cast<int>(variable->get_measurements_count()) <= row) {
            variable->add_measurement(0.0);
        }
        variable->set_measurement(row, doubleValue);
    }

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
    endResetModel();
}
