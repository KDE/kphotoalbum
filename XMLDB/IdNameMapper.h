#ifndef IDNAMEMAPPER_H
#define IDNAMEMAPPER_H

#include "DB/RawId.h"
#include <QMap>
#include <QString>

namespace DB
{
class IdNameMapper
{
public:
    IdNameMapper();
    void add( const QString& fileName );
    void remove( DB::RawId id );
    void remove( const QString& fileName );

    bool exists(const QString & filename) const;
    
    DB::RawId operator[](const QString& ) const;
    QString operator[]( DB::RawId ) const;
private:
    QMap<DB::RawId, QString> _idTofileName;
    QMap<QString, DB::RawId> _fileNameToId;
    int _maxId;
};

}


#endif /* IDNAMEMAPPER_H */

