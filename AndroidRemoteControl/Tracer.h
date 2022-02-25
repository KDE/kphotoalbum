#pragma once
#include <QDebug>
#include <QString>

class Tracer
{
public:
    Tracer(const QString &fileName)
        : m_fileName(fileName)
    {
        qDebug().noquote() << QString("%1Enter").arg("", level, ' ') << fileName;
        level += 4;
    }
    ~Tracer()
    {
        level -= 4;
        qDebug().noquote() << QString("%1Leave").arg("", level, ' ') << m_fileName;
    }

private:
    QString m_fileName;
    static int level;
};

#define TRACE // Tracer dummy(__FUNCTION__);
