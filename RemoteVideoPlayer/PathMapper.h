#pragma once

#include <QObject>
#include <QVector>

class PathMapper : public QObject
{
    Q_OBJECT

public:
    static PathMapper &instance();
    QString map(const QString linuxPath);
    void removeMapping(const QString &linuxPath, const QString &hostPath);
    void addMapping(const QString &linuxPath, const QString &hostPath);
    bool configurePath(const QString &linuxPath, const QString &hostPath);

    struct Mapping {
        QString linuxPath;
        QString hostPath;
        bool operator==(const Mapping &other)
        {
            return other.linuxPath == linuxPath && other.hostPath == hostPath;
        }
    };
    QVector<Mapping> mappings() const;

signals:
    void setupChanged();

private:
    PathMapper();

    QVector<Mapping> m_mappings;
};
