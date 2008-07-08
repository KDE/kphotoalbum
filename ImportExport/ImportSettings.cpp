#include "ImportSettings.h"

void ImportExport::ImportSettings::setSelectedImages( const DB::ImageInfoList& list)
{
    m_selectedImages = list;
}

DB::ImageInfoList ImportExport::ImportSettings::selectedImages() const
{
    return m_selectedImages;
}

void ImportExport::ImportSettings::setDestination( const QString& destination )
{
    m_destination = destination;
}

QString ImportExport::ImportSettings::destination() const
{
    return m_destination;
}

void ImportExport::ImportSettings::setExternalSource( bool b )
{
    m_externalSource = b;
}

bool ImportExport::ImportSettings::externalSource() const
{
    return m_externalSource;
}

void ImportExport::ImportSettings::setKimFile( const KUrl& kimFile )
{
    m_kimFile = kimFile;
}

KUrl ImportExport::ImportSettings::kimFile() const
{
    return m_kimFile;
}

void ImportExport::ImportSettings::setBaseURL( const KUrl& url )
{
    m_baseURL = url;
}

KUrl ImportExport::ImportSettings::baseURL() const
{
    return m_baseURL;
}

void ImportExport::ImportSettings::setImportMatchers( const ImportMatchers& matchers )
{
    m_importMatchers = matchers;
}

const ImportExport::ImportMatchers ImportExport::ImportSettings::importMatchers() const
{
    return m_importMatchers;
}

ImportExport::ImportSettings::ImportAction ImportExport::ImportSettings::importAction( const QString& item )
{
    return m_actions[item];
}

void ImportExport::ImportSettings::setImportActions( const QMap<QString, ImportAction>& actions )
{
    m_actions = actions;
}
