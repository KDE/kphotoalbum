#ifndef XMLDB_FILEREADER_H
#define XMLDB_FILEREADER_H

#include <qdom.h>
#include "DB/ImageInfoPtr.h"

namespace XMLDB
{
class Database;

class FileReader
{

public:
    FileReader( Database* db ) : _db( db ) {}
    void read( const QString& configFile );
    static QString unescape( const QString& );

protected:
    void readTopNodeInConfigDocument( const QString& configFile, QDomElement top, QDomElement* options, QDomElement* images,
                                      QDomElement* blockList, QDomElement* memberGroups );
    void loadCategories( const QDomElement& elm );
    void loadImages( const QDomElement& images );
    void loadBlockList( const QDomElement& blockList );
    void loadMemberGroups( const QDomElement& memberGroups );
    DB::ImageInfoPtr load( const QString& filename, QDomElement elm );
    QDomElement readConfigFile( const QString& configFile );

    void createSpecialCategories();

    void checkIfImagesAreSorted();
    void checkIfAllImagesHasSizeAttributes();
    void checkAndWarnAboutVersionConflict();

private:
    Database* _db;
    int _fileVersion;
};

}

#endif /* XMLDB_FILEREADER_H */

