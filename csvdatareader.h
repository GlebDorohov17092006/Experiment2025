#ifndef CSVDATAREADER_H
#define CSVDATAREADER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QStringList>
#include <QDebug>

struct DataPoint {
    double x;
    double y;
    QString timestamp;
};

class CSVDataReader : public QObject
{
    Q_OBJECT

public:
    explicit CSVDataReader(QObject *parent = nullptr);
    bool readData(const QString &filename);
    QVector<DataPoint> getData() const { return m_data; }
    QStringList getHeaders() const { return m_headers; }
    bool hasData() const { return !m_data.isEmpty(); }

private:
    QVector<DataPoint> m_data;
    QStringList m_headers;
    QString m_filename;

    bool parseLine(const QString &line, int lineNumber);
    double toDouble(const QString &str, bool &ok);
};

#endif // CSVDATAREADER_H
