#ifndef IDNAMEMAPPER_H
#define IDNAMEMAPPER_H

#include <QMap>
#include <QString>

namespace DB
{
class IdNameMapper
{
public:
    IdNameMapper();
    void add( const QString& fileName );
    void remove( int id );
    void remove( const QString& fileName );

    int operator[](const QString& ) const;
    QString operator[]( int ) const;
private:
    QMap<int,QString> _idTofileName;
    QMap<QString,int> _fileNameToId;
    int _maxId;
};

}


#endif /* IDNAMEMAPPER_H */

