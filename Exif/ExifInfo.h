#ifndef EXIF_H
#define EXIF_H
#include <qmap.h>
#include <qstringlist.h>
#include "set.h"

class ExifInfo {
public:
    ExifInfo();
    static ExifInfo* instance();
    QMap<QString, QString> info( const QString& fileName, Set<QString> wantedKeys, bool fullName );
    QMap<QString, QString> infoForViewer( const QString& fileName );
    QMap<QString, QString> infoForDialog( const QString& fileName );
    Set<QString> availableKeys();
    Set<QString> standardKeys();

private:
    static ExifInfo* _instance;
    Set<QString> _keys;
};

#endif /* EXIF_H */

