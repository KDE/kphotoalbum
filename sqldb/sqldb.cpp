// HINT: distinct

#include "sqldb.h"
#include <membermap.h>
#include <qsqldatabase.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <stdlib.h>
#include <categorycollection.h>
#include <qsqlerror.h>

const QString imageInfoAttributes = "label, description, dayFrom, monthFrom, yearFrom, dayTo, monthTo, "
                                    "yearTo, hour, minute, second, angle, md5sum, width, height";

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

QStringList SQLDB::SQLDB::search( const ImageSearchInfo& info, bool /*requireOnDisk*/ ) const
{
    info.debugMatcher();
    return filesMatchingQuery( info );
}

void SQLDB::SQLDB::renameOptionGroup( const QString& /*oldName*/, const QString /*newName*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::renameOptionGroup( const QString& oldName, const QString newName )" );
}

QMap<QString,int> SQLDB::SQLDB::classify( const ImageSearchInfo& info, const QString& category )
{
    QStringList matches = filesMatchingQuery( info );
    QMap<QString,int> result;

    for( QStringList::ConstIterator it = matches.begin(); it != matches.end(); ++it ) {
        QSqlQuery categoryQuery;
        if ( !categoryQuery.exec( QString::fromLatin1("SELECT value from imagecategoryinfo WHERE filename=\"%1\" and category=\"%2\"" )
                                  .arg( *it ).arg( category ) ) )
            showError( categoryQuery.lastError() );

        while ( categoryQuery.next() )
            result[ categoryQuery.value(0).toString() ]++;
    }
    return result;
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

ImageInfo* SQLDB::SQLDB::info( const QString& fileName ) const
{
    QString file = fileName;
    if ( fileName.startsWith( Options::instance()->imageDirectory() ) )
        file = fileName.mid( Options::instance()->imageDirectory().length() );

    static QMap<QString,ImageInfo*> map;
    if ( map.contains( file ) )
        return map[file];

    QSqlQuery query;
    if ( !query.exec( QString::fromLatin1( "SELECT %1 FROM imageinfo where fileName=\"%2\"" ).arg( imageInfoAttributes ).arg( file ) ) )
        showError( query.lastError() );

    Q_ASSERT( query.numRowsAffected() == 1 );

    query.next();
    QString label = query.value(0).toString();
    QString description = query.value( 1 ).toString();
    int dayFrom = query.value( 2 ).toInt();
    int monthFrom = query.value( 3 ).toInt();
    int yearFrom = query.value( 4 ).toInt();
    int dayTo = query.value( 5 ).toInt();
    int monthTo = query.value( 6 ).toInt();
    int yearTo = query.value( 7 ).toInt();
    int hour = query.value( 8 ).toInt();
    int minute = query.value( 9 ).toInt();
    int second = query.value( 10 ).toInt();
    int angle = query.value( 11 ).toInt();
    QString     md5sum = query.value( 12 ).toString();
    int width = query.value( 13 ).toInt();
    int heigh = query.value( 14 ).toInt();

    // PENDING(blackie) where will this be deleted?
    ImageInfo* info = new ImageInfo( file, label, description, ImageDate( dayFrom, monthFrom, yearFrom, hour, minute, second ),
                                     ImageDate( dayTo, monthTo, yearTo ), angle, md5sum, QSize( width, heigh ) );


    if ( !query.exec( QString::fromLatin1( "SELECT category, value FROM imagecategoryinfo WHERE fileName=\"%1\"" )
                      .arg( file ) ) )
        showError( query.lastError() );

    while ( query.next() ) {
        info->addOption( query.value(0).toString(), query.value(1).toString() );
    }

    map.insert( file, info );
    return info;
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
    return &_md5map;
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

    QStringList categories;
    while ( categoriesQuery.next() ) {
        QString category = categoriesQuery.value(0).toString();
        int viewtype = categoriesQuery.value(1).toInt();
        int viewsize = categoriesQuery.value(2).toInt();
        QString icon = categoriesQuery.value(3).toString();
        bool show = categoriesQuery.value(4).toBool();

        Category* cat = _categoryCollection.categoryForName( category ); // Special Categories are already created.
        if ( !cat ) {
            cat = new Category( category, icon, (Category::ViewSize) viewsize, (Category::ViewType) viewtype, show );
            _categoryCollection.addCategory( cat );
        }
        // PENDING(blackie) else set the values for icons, size, type, and show
    }

    QSqlQuery categoryValuesQuery;
    if ( !categoryValuesQuery.exec( "select distinct category, value from imagecategoryinfo" ) )
        qFatal("Unable to exec query");

    while (categoryValuesQuery.next() ) {
        QString category = categoryValuesQuery.value(0).toString();
        QString value = categoryValuesQuery.value(1).toString();
        Category* cat = _categoryCollection.categoryForName( category );
        if ( cat )
            cat->addItem( value );
    }
}

CategoryCollection* SQLDB::SQLDB::categoryCollection()
{
    return &_categoryCollection;
}

QStringList SQLDB::SQLDB::allImages() const
{
    QSqlQuery sortOrderQuery;
    if ( !sortOrderQuery.exec( "SELECT fileName FROM sortorder" ) ) // PENDING(blackie)  ORDER by idx
        qFatal( "Unable to exe query" );

    QStringList result;
    while (sortOrderQuery.next() )
        result.append( Options::instance()->imageDirectory() + sortOrderQuery.value(0).toString() );
    return result;
}

void SQLDB::SQLDB::showError( const QSqlError& error ) const
{
    qFatal( "Error running query: %s,%s", error.driverText().latin1(), error.databaseText().latin1() );
}

QStringList SQLDB::SQLDB::filesMatchingQuery( const ImageSearchInfo& info ) const
{
    QString queryTxt = QString::fromLatin1( "SELECT distinct fileName FROM imagecategoryinfo" );
    if ( !info.isNull() ) {
        queryTxt += QString::fromLatin1( " WHERE " ) + info.toSQLQuery();
    }

    queryTxt = QString::fromLatin1( "SELECT q1.fileName FROM imagecategoryinfo q1, imagecategoryinfo q2 WHERE q1.fileName = q2.fileName and q1.category = \"Persons\" and q1.value = \"Jesper\" and q2.category = \"Persons\" and q2.value = \"Anne Helene\"" );

    QSqlQuery query;
    if ( !query.exec( queryTxt ) )
        showError( query.lastError() );

    QStringList result;
    while ( query.next() )
        result.append( Options::instance()->imageDirectory() + query.value(0).toString() );
    return result;
}

