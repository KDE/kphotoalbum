#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

#include <imageinfo.h>

class Export {

public:
    static void imageExport( const ImageInfoList& list );
    static QCString createIndexXML( const ImageInfoList&, const QMap<QString, QString>& );
};


#endif /* IMPORTEXPORT_H */

