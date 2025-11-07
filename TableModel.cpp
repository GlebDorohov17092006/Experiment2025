#include "TableModel.h"
#include "Variable.h"
#include "Instrument.h"

TableModel::TableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_experiment(nullptr)
{
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_experiment || m_variables.empty())
        return 0;

    size_t maxRows = 0;
    for (const auto& var : m_variables) {
        maxRows = std::max(maxRows, var->get_measurements_count());
    }
    return static_cast<int>(maxRows);
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_experiment)
        return 0;

    return static_cast<int>(m_variables.size());
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_experiment || m_variables.empty())
        return QVariant();

    int col = index.column();
    int row = index.row();

    if (col < 0 || col >= static_cast<int>(m_variables.size()) || row < 0)
        return QVariant();

    const auto& variable = m_variables[col];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (row < static_cast<int>(variable->get_measurements_count())) {
            double value = variable->get_measurement(row);

            if (role == Qt::DisplayRole) {
                try {
                    double error = variable->get_error_instrument(0, value);
                    if (error > 0) {
                        return QString("%1 ± %2").arg(value, 0, 'f', 3).arg(error, 0, 'f', 3);
                    }
                } catch (const std::exception&) {
                    // Если ошибка не может быть вычислена, показываем только значение
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
    if (role != Qt::DisplayRole || !m_experiment)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if (section >= 0 && section < static_cast<int>(m_variables.size())) {
            const auto& variable = m_variables[section];
            QString name = QString::fromStdString(variable->get_name_tables());
            QString tag = QString::fromStdString(variable->get_name_calculated());

            if (!tag.isEmpty()) {
                return QString("%1\n(%2)").arg(name).arg(tag);
            }
            return name;
        }
    } else {
        return section + 1;
    }

    return QVariant();
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || !m_experiment || m_variables.empty() || role != Qt::EditRole)
        return false;

    int col = index.column();
    int row = index.row();

    if (col < 0 || col >= static_cast<int>(m_variables.size()) || row < 0)
        return false;

    auto& variable = m_variables[col];

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

void TableModel::setExperiment(Experiment* experiment)
{
    beginResetModel();
    m_experiment = experiment;
    refreshData();
    endResetModel();
}

void TableModel::refreshData()
{
    m_variables.clear();

    if (!m_experiment)
        return;

    size_t varCount = m_experiment->get_variables_count();
    for (size_t i = 0; i < varCount; ++i) {
        m_variables.push_back(m_experiment->get_variable(i));
    }
}
