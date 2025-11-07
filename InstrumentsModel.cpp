#include "InstrumentsModel.h"
#include "Instrument.h"
#include "AbsoluteInstrument.h"
#include "RelativeInstrument.h"
#include "CombinedInstrument.h"

InstrumentsModel::InstrumentsModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_instruments(nullptr)
{
}

int InstrumentsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_instruments)
        return 0;

    return static_cast<int>(m_instruments->size());
}

int InstrumentsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 3;
}

QVariant InstrumentsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_instruments || m_instruments->empty())
        return QVariant();

    int row = index.row();
    int col = index.column();

    if (row < 0 || row >= static_cast<int>(m_instruments->size()))
        return QVariant();

    const auto& instrument = (*m_instruments)[row];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (col) {
        case 0:
            return QString::fromStdString(instrument->get_name());
        case 1:
            if (dynamic_cast<AbsoluteInstrument*>(instrument.get())) {
                return "Абсолютная";
            } else if (dynamic_cast<RelativeInstrument*>(instrument.get())) {
                return "Относительная";
            } else if (dynamic_cast<CombinedInstrument*>(instrument.get())) {
                return "Комбинированная";
            } else {
                return "Неизвестный";
            }
        case 2:
            return QString::number(instrument->get_error(0, 0), 'f', 3);
        }
    }

    return QVariant();
}

QVariant InstrumentsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return "Прибор";
        case 1: return "Тип ошибки";
        case 2: return "Погрешность";
        default: return QVariant();
        }
    }

    return QVariant();
}

bool InstrumentsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || !m_instruments || m_instruments->empty() || role != Qt::EditRole)
        return false;

    int row = index.row();
    int col = index.column();

    if (row < 0 || row >= static_cast<int>(m_instruments->size()))
        return false;

    auto& instrument = (*m_instruments)[row];

    switch (col) {
    case 0:
        instrument->set_name(value.toString().toStdString());
        break;
    case 2:
        instrument->set_error(0, value.toDouble());
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, {role});
    return true;
}

Qt::ItemFlags InstrumentsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);

    if (index.isValid() && index.column() != 1) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

void InstrumentsModel::setInstruments(std::vector<std::shared_ptr<Instrument>>* instruments)
{
    beginResetModel();
    m_instruments = instruments;
    endResetModel();
}

void InstrumentsModel::refreshData()
{
    beginResetModel();
    endResetModel();
}
