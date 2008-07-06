#ifndef IMPORTSETTINGS_H
#define IMPORTSETTINGS_H

#include <kurl.h>
#include "DB/ImageInfoList.h"

namespace ImportExport
{

class ImportSettings
{
public:
    void setSelectedImages( const DB::ImageInfoList& );
    DB::ImageInfoList selectedImages() const;

    void setDestination( const QString& );
    QString destination() const;

    void setExternalSource( bool b );
    bool externalSource() const;

    void setKimFile( const KUrl& kimFile );
    KUrl kimFile() const;

    void setBaseURL( const KUrl& url );
    KUrl baseURL() const;

private:
    DB::ImageInfoList m_selectedImages;
    QString m_destination;
    bool m_externalSource;
    KUrl m_kimFile;
    KUrl m_baseURL;
};

}

#endif /* IMPORTSETTINGS_H */

