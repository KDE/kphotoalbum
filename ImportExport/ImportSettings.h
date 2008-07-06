#ifndef IMPORTSETTINGS_H
#define IMPORTSETTINGS_H

#include <kurl.h>
#include "DB/ImageInfoList.h"
#include "ImportMatcher.h"
namespace ImportExport
{

/**
 * The class contains all the data that is transported between the
 * ImportDialog, and the ImportHandler. The purpose of this class is to
 * decouple the above two.
 */
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

    void setImportMatchers( const ImportMatchers* matchers );
    const ImportMatchers* importMatchers() const;

private:
    DB::ImageInfoList m_selectedImages;
    QString m_destination;
    bool m_externalSource;
    KUrl m_kimFile;
    KUrl m_baseURL;
    const ImportMatchers* m_importMatchers;
};

}

#endif /* IMPORTSETTINGS_H */

