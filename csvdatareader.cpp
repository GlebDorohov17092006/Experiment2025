#include "csvdatareader.h"

CSVDataReader::CSVDataReader(QObject *parent) : QObject(parent)
{
}

bool CSVDataReader::readData(const QString &filename)
{
    m_data.clear();
    m_headers.clear();
    m_filename = filename;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл:" << filename;
        return false;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");

    if (!in.atEnd()) {
        QString headerLine = in.readLine();
        m_headers = headerLine.split(';', Qt::SkipEmptyParts);
        qDebug() << "Заголовки:" << m_headers;
    }

    int lineNumber = 1;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!parseLine(line, lineNumber)) {
            qDebug() << "Ошибка парсинга строки" << lineNumber;
        }
        lineNumber++;
    }

    file.close();
    qDebug() << "Прочитано точек данных:" << m_data.size();
    return !m_data.isEmpty();
}

bool CSVDataReader::parseLine(const QString &line, int lineNumber)
{
    QStringList parts = line.split(';', Qt::SkipEmptyParts);
    if (parts.size() < 2) {
        return false;
    }

    DataPoint point;
    bool ok1, ok2;

    point.x = toDouble(parts[0], ok1);

    point.y = toDouble(parts[1], ok2);

    if (parts.size() > 2) {
        point.timestamp = parts[2];
    }

    if (ok1 && ok2) {
        m_data.append(point);
        return true;
    }

    return false;
}

double CSVDataReader::toDouble(const QString &str, bool &ok)
{
    QString cleaned = str.trimmed().replace(',', '.');
    return cleaned.toDouble(&ok);
}
