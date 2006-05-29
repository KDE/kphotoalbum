#ifndef XMLHANDLER_H
#define XMLHANDLER_H
#include <qcstring.h>
#include <qstringlist.h>
#include <qstring.h>
#include "Export.h"
#include "Utilities/Util.h"
#include "DB/ImageInfoPtr.h"
#include <qdom.h>

namespace ImportExport
{
class XMLHandler
{
public:
    QCString createIndexXML( const QStringList& images, const QString& baseUrl, ImageFileLocation location,
                             const Utilities::UniqNameMap& nameMap );

protected:
    QDomElement save( QDomDocument doc, const DB::ImageInfoPtr& info );
    void writeCategories( QDomDocument doc, QDomElement elm, const DB::ImageInfoPtr& info );
};

}

#endif /* XMLHANDLER_H */

