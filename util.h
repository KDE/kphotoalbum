#ifndef UTIL_H
#define UTIL_H
#include <qdom.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include "options.h"
class ImageInfo;

class Util {
public:
    static bool writeOptions( QDomDocument doc,  QDomElement elm, QMap<QString, QStringList>& options,
                              QMap<QString,Options::OptionGroupInfo>* optionGroupInfo );
    static void readOptions( QDomElement elm, QMap<QString, QStringList>* options,
                             QMap<QString,Options::OptionGroupInfo>* optionGroupInfo );
    static QString createInfoText( ImageInfo* info, QMap<int, QPair<QString,QString> >* );
    static void checkForBackupFile( const QString& realName, const QString& backupName );
    static bool ctrlKeyDown();
};


#endif /* UTIL_H */

