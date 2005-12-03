#ifndef EXIF_H
#define EXIF_H
#include <qmap.h>
#include <qstringlist.h>
#include "set.h"

namespace Exif {

class Info {
public:
    Info();
    static Info* instance();
    QMap<QString, QString> info( const QString& fileName, Set<QString> wantedKeys, bool fullName );
    QMap<QString, QString> infoForViewer( const QString& fileName );
    QMap<QString, QString> infoForDialog( const QString& fileName );
    Set<QString> availableKeys();
    Set<QString> standardKeys();

private:
    static Info* _instance;
    Set<QString> _keys;
};

}

#endif /* EXIF_H */

