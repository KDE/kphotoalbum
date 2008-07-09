#ifndef IMPORTSETTINGS_H
#define IMPORTSETTINGS_H

#include <kurl.h>
#include "DB/ImageInfoList.h"
namespace ImportExport
{


class CategoryMatchSetting
{
public:
    CategoryMatchSetting( const QString& DBCategoryName, const QString& XMLFileCategoryName )
        : m_XMLCategoryName( XMLFileCategoryName ), m_DBCategoryName(DBCategoryName ) {}
    void add( const QString& DBFileNameItem, const QString& XMLFileNameItem );

    QString XMLCategoryName() const;
    QString DBCategoryName() const;
    const QMap<QString, QString>& XMLtoDB() const;

private:
    QString m_XMLCategoryName;
    QString m_DBCategoryName;
    QMap<QString, QString> m_XMLtoDB;
};

/**
 * The class contains all the data that is transported between the
 * ImportDialog, and the ImportHandler. The purpose of this class is to
 * decouple the above two.
 */
class ImportSettings
{
public:
    enum ImportAction { Replace = 1, Keep = 2, Merge = 3 };

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

    void setImportActions( const QMap<QString, ImportAction>& actions );
    ImportAction importAction( const QString& item );

    void addCategoryMatchSetting( const CategoryMatchSetting& );
    QList<CategoryMatchSetting> categoryMatchSetting() const;

private:
    DB::ImageInfoList m_selectedImages;
    QString m_destination;
    bool m_externalSource;
    KUrl m_kimFile;
    KUrl m_baseURL;
    QMap<QString, ImportAction> m_actions;
    QList<CategoryMatchSetting> m_categoryMatchSettings;
};

}

#endif /* IMPORTSETTINGS_H */

