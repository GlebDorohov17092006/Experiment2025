#pragma once

#include <QAbstractTableModel>
#include <vector>
#include <memory>

class Instrument;

class InstrumentsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit InstrumentsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setInstruments(std::vector<std::shared_ptr<Instrument>>* instruments);
    void refreshData();

signals:
    void instrumentNameChanged(); // Добавляем сигнал

private:
    std::vector<std::shared_ptr<Instrument>>* m_instruments;
};
