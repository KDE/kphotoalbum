#ifndef UTIL_H
#define UTIL_H
#include <qdom.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

class Util {
public:
    static void writeOptions( QDomDocument doc,  QDomElement elm, QMap<QString, QStringList>& options );
    static void readOptions( QDomElement elm, QMap<QString, QStringList>* options );
};


#endif /* UTIL_H */

