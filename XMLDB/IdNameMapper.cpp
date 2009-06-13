#include "IdNameMapper.h"

void DB::IdNameMapper::add( const QString& fileName )
{
    const DB::RawId id(++_maxId);
    Q_ASSERT(id != DB::RawId());
    _idTofileName.insert( id, fileName );
    _fileNameToId.insert( fileName, id );
    Q_ASSERT( !fileName.startsWith(QLatin1String("/")));
}

DB::RawId DB::IdNameMapper::operator[](const QString& fileName ) const
{
    Q_ASSERT( !fileName.startsWith( QLatin1String("/") ) );
    Q_ASSERT( _fileNameToId.contains( fileName ) );
    return _fileNameToId[fileName];
}

QString DB::IdNameMapper::operator[]( DB::RawId id ) const
{
    Q_ASSERT( _idTofileName.contains( id ) );
    return _idTofileName[id];
}

void DB::IdNameMapper::remove( DB::RawId id )
{
    Q_ASSERT( _idTofileName.contains( id ) );
    _fileNameToId.remove( _idTofileName[id] );
    _idTofileName.remove( id );
}

void DB::IdNameMapper::remove( const QString& fileName )
{
    Q_ASSERT( _fileNameToId.contains( fileName ) );
    Q_ASSERT( !fileName.startsWith( QLatin1String("/") ) );
    _idTofileName.remove( _fileNameToId[fileName] );
    _fileNameToId.remove( fileName );
}

DB::IdNameMapper::IdNameMapper()
    :_maxId(0)
{
}

bool DB::IdNameMapper::exists(const QString& fileName ) const
{
  return _fileNameToId.find(fileName) != _fileNameToId.end();
}
