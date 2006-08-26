#ifndef XMLDB_FILESAVER_H
#define XMLDB_FILESAVER_H

#include <qstring.h>
#include <qdom.h>
#include "DB/ImageInfoPtr.h"

namespace XMLDB
{
class Database;

class FileWriter
{
public:
    FileWriter( Database* db ) :_db(db) {}
    void save( const QString& fileName, bool isAutoSave );
    static QString escape( const QString& );

protected:
    void saveCategories( QDomDocument doc, QDomElement top );
    void saveImages( QDomDocument doc, QDomElement top );
    void saveBlockList( QDomDocument doc, QDomElement top );
    void saveMemberGroups( QDomDocument doc, QDomElement top );
    void add21CompatXML( QDomElement& top );
    QDomElement save( QDomDocument doc, const DB::ImageInfoPtr& info );
    void writeCategories( QDomDocument doc,  QDomElement elm, const DB::ImageInfoPtr& info );
    void writeCategoriesCompressed( QDomElement& elm, const DB::ImageInfoPtr& info );
    bool shouldSaveCategory( const QString& categoryName ) const;

private:
    Database* _db;
};

}


#endif /* XMLDB_FILESAVER_H */

