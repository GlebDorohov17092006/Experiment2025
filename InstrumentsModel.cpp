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
            if (instrument->get_name() == "(нет инструмента)") {
                return "Без инструмента";
            } else if (dynamic_cast<AbsoluteInstrument*>(instrument.get())) {
                return "Абсолютная";
            } else if (dynamic_cast<RelativeInstrument*>(instrument.get())) {
                return "Относительная";
            } else {
                return "Неизвестный";
            }
        case 2:
            if (instrument->get_name() == "(нет инструмента)") {
                return "0.000";
            } else if (auto absInstr = dynamic_cast<AbsoluteInstrument*>(instrument.get())) {
                return QString::number(absInstr->get_error(0, 0), 'f', 3);
            } else if (auto relInstr = dynamic_cast<RelativeInstrument*>(instrument.get())) {
                return QString::number(relInstr->get_error(0, 1) * 100, 'f', 1) + "%";
            }
            return "0.000";
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

    // Запрещаем редактирование инструмента "Нет инструмента"
    if (instrument->get_name() == "(нет инструмента)") {
        return false;
    }

    switch (col) {
    case 0:
        instrument->set_name(value.toString().toStdString());
        // После изменения названия инструмента, обновляем делегат в главном окне
        emit instrumentNameChanged(); // Добавляем сигнал
        break;
    case 1:
        // Обработка изменения типа инструмента через делегат
        {
            QString newType = value.toString();
            std::shared_ptr<Instrument> newInstrument;

            if (newType == "Абсолютная") {
                double oldError = 0.0;
                if (auto absInstr = dynamic_cast<AbsoluteInstrument*>(instrument.get())) {
                    oldError = absInstr->get_error(0, 0);
                } else if (auto relInstr = dynamic_cast<RelativeInstrument*>(instrument.get())) {
                    oldError = relInstr->get_error(0, 1.0);
                }
                newInstrument = std::make_shared<AbsoluteInstrument>(instrument->get_name(), oldError);
            } else if (newType == "Относительная") {
                double oldError = 0.0;
                if (auto absInstr = dynamic_cast<AbsoluteInstrument*>(instrument.get())) {
                    oldError = absInstr->get_error(0, 0) / 100.0;
                } else if (auto relInstr = dynamic_cast<RelativeInstrument*>(instrument.get())) {
                    oldError = relInstr->get_error(0, 1.0);
                }
                newInstrument = std::make_shared<RelativeInstrument>(instrument->get_name(), oldError);
            }

            if (newInstrument) {
                (*m_instruments)[row] = newInstrument;
                emit instrumentNameChanged(); // И здесь тоже
            }
        }
        break;
    case 2:
        if (auto absInstr = dynamic_cast<AbsoluteInstrument*>(instrument.get())) {
            absInstr->set_error(0, value.toDouble());
        } else if (auto relInstr = dynamic_cast<RelativeInstrument*>(instrument.get())) {
            QString strValue = value.toString();
            if (strValue.endsWith('%')) {
                strValue.chop(1);
                bool ok;
                double percentValue = strValue.toDouble(&ok);
                if (ok) {
                    relInstr->set_error(0, percentValue / 100.0);
                }
            } else {
                relInstr->set_error(0, value.toDouble());
            }
        }
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

    if (index.isValid()) {
        int row = index.row();
        // Запрещаем редактирование инструмента "Нет инструмента"
        if (m_instruments && row < static_cast<int>(m_instruments->size())) {
            const auto& instrument = (*m_instruments)[row];
            if (instrument->get_name() == "(нет инструмента)") {
                flags &= ~Qt::ItemIsEditable; // Убираем флаг редактирования
            } else {
                flags |= Qt::ItemIsEditable;
            }
        }
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
