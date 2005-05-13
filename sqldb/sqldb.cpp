// HINT: distinct

#include "sqldb.h"
#include <membermap.h>
#include <qsqldatabase.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <stdlib.h>
#include <categorycollection.h>

SQLDB::SQLDB::SQLDB()
{
    if ( !QSqlDatabase::isDriverAvailable( "QMYSQL3" ) ) {
        // PENDING(blackie) better message
        KMessageBox::sorry( 0, i18n("The MySQL driver did not seem to be compiled into your Qt" ) );
        exit(-1);
    }

    openDatabase();
    loadMemberGroups();
    loadCategories();
}

int SQLDB::SQLDB::totalCount() const
{
    qDebug("NYI: int SQLDB::SQLDB::totalCount() const" );
    return 24;
}

QStringList SQLDB::SQLDB::search( const ImageSearchInfo&, bool /*requireOnDisk*/ ) const
{
    qDebug("NYI: QStringList SQLDB::SQLDB::search( const ImageSearchInfo&, bool requireOnDisk ) const" );
    return QStringList();
}

int SQLDB::SQLDB::count( const ImageSearchInfo& /*info*/ )
{
    qDebug("NYI: int SQLDB::SQLDB::count( const ImageSearchInfo& info )" );
    return 0;
}

void SQLDB::SQLDB::renameOptionGroup( const QString& /*oldName*/, const QString /*newName*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::renameOptionGroup( const QString& oldName, const QString newName )" );
}

QMap<QString,int> SQLDB::SQLDB::classify( const ImageSearchInfo& /*info*/, const QString &/*group*/ )
{
    qDebug("NYI: QMap<QString,int> SQLDB::SQLDB::classify( const ImageSearchInfo& info, const QString &group )" );
    return QMap<QString,int>();
}

ImageInfoList& SQLDB::SQLDB::imageInfoList()
{
    qDebug("NYI: ImageInfoList& SQLDB::SQLDB::imageInfoList()" );
    static ImageInfoList list;
    return list;
}

QStringList SQLDB::SQLDB::images()
{
    qDebug("NYI: QStringList SQLDB::SQLDB::images()" );
    return QStringList();
}

void SQLDB::SQLDB::addImages( const ImageInfoList& /*images*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::addImages( const ImageInfoList& images )" );
}

void SQLDB::SQLDB::addToBlockList( const QStringList& /*list*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::addToBlockList( const QStringList& list )" );
}

bool SQLDB::SQLDB::isBlocking( const QString& /*fileName*/ )
{
    qDebug("NYI: bool SQLDB::SQLDB::isBlocking( const QString& fileName )" );
    return true;
}

void SQLDB::SQLDB::deleteList( const QStringList& /*list*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::deleteList( const QStringList& list )" );
}

ImageInfo* SQLDB::SQLDB::info( const QString& /*fileName*/ ) const
{
    qDebug("NYI: ImageInfo* SQLDB::SQLDB::info( const QString& fileName ) const" );
    return 0;
}

const MemberMap& SQLDB::SQLDB::memberMap()
{
    return _members;
}

void SQLDB::SQLDB::setMemberMap( const MemberMap& map )
{
    _members = map;
}

void SQLDB::SQLDB::save( const QString& /*fileName*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::save( const QString& fileName )" );
}

MD5Map* SQLDB::SQLDB::md5Map()
{
    qDebug("NYI: MD5Map* SQLDB::SQLDB::md5Map()" );
    return 0;
}

void SQLDB::SQLDB::sortAndMergeBackIn( const QStringList& /*fileList*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::sortAndMergeBackIn( const QStringList& fileList )" );
}

void SQLDB::SQLDB::renameOption( Category* /*category*/, const QString& /*oldName*/, const QString& /*newName*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::renameOption( Category* category, const QString& oldName, const QString& newName )" );
}

void SQLDB::SQLDB::deleteOption( Category* /*category*/, const QString& /*option*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::deleteOption( Category* category, const QString& option )" );
}

void SQLDB::SQLDB::lockDB( bool /*lock*/, bool /*exclude*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::lockDB( bool lock, bool exclude )" );
}

void SQLDB::SQLDB::slotReread( const QStringList& /*list*/, int /*mode*/)
{
    qDebug("NYI: void SQLDB::SQLDB::slotReread( const QStringList& list, int mode)" );
}


void SQLDB::SQLDB::openDatabase()
{
    QSqlDatabase* database = QSqlDatabase::addDatabase( "QMYSQL3" );
    if ( database == 0 ) {
        qFatal("What?!");
    }

    database->setDatabaseName( "kimdaba" );
    database->setUserName("root");
    if ( !database->open() )
        qFatal("Couldn't open db");
}

void SQLDB::SQLDB::loadMemberGroups()
{
    QSqlQuery membersQuery;
    if (!membersQuery.exec( "SELECT groupname, category, member FROM membergroup" ) )
        qFatal("Couldn't exec query");

    while ( membersQuery.next() ) {
        QString group = membersQuery.value(0).toString();
        QString category = membersQuery.value(1).toString();
        QString member = membersQuery.value(2).toString();
        _members.addMemberToGroup( category, group, member );
    }
}

void SQLDB::SQLDB::loadCategories()
{
    QSqlQuery categoriesQuery;
    if ( !categoriesQuery.exec( "SELECT category, viewtype, viewsize, icon, showIt FROM categorysetup" ) )
        qFatal("Could not run query");

    CategoryCollection* collection = CategoryCollection::instance();
    QStringList categories;
    while ( categoriesQuery.next() ) {
        QString category = categoriesQuery.value(0).toString();
        int viewtype = categoriesQuery.value(1).toInt();
        int viewsize = categoriesQuery.value(2).toInt();
        QString icon = categoriesQuery.value(3).toString();
        bool show = categoriesQuery.value(4).toBool();

        Category* cat = collection->categoryForName( category ); // Special Categories are already created.
        if ( !cat ) {
            cat = new Category( category, icon, (Category::ViewSize) viewsize, (Category::ViewType) viewtype, show );
            collection->addCategory( cat );
        }
        // PENDING(blackie) else set the values for icons, size, type, and show
    }

    QSqlQuery categoryValuesQuery;
    if ( !categoryValuesQuery.exec( "select distinct category, value from imagecategoryinfo" ) )
        qFatal("Unable to exec query");

    while (categoryValuesQuery.next() ) {
        QString category = categoryValuesQuery.value(0).toString();
        QString value = categoryValuesQuery.value(1).toString();
        Category* cat = collection->categoryForName( category );
        if ( cat )
            cat->addItem( value );
    }



}
