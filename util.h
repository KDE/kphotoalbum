#ifndef UTIL_H
#define UTIL_H
#include <qdom.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
class ImageInfo;

class Util {
public:
    static bool writeOptions( QDomDocument doc,  QDomElement elm, QMap<QString, QStringList>& options );
    static void readOptions( QDomElement elm, QMap<QString, QStringList>* options );
    static QString createInfoText( ImageInfo* info );
};


#endif /* UTIL_H */

