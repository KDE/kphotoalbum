// HINT: distinct

#include "sqldb.h"
#include <membermap.h>
#include <qsqldatabase.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <stdlib.h>
#include <categorycollection.h>
#include <qsqlerror.h>
#include "query.h"
#include <imageinfo.h>
#include <util.h>

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
    QString queryString = QString::fromLatin1( "SELECT * from sortorder" );
    QSqlQuery query;
    if ( !query.exec( queryString ) )
        showError( query.lastError(), queryString );
    return query.numRowsAffected();
}

QStringList SQLDB::SQLDB::search( const ImageSearchInfo& info, bool /*requireOnDisk*/ ) const
{
    QStringList matches = filesMatchingQuery( info );
    QStringList result;
    for( QStringList::Iterator it = matches.begin(); it != matches.end(); ++it ) {
        result.append( Options::instance()->imageDirectory() + *it );
    }
    return result;
}

void SQLDB::SQLDB::renameOptionGroup( const QString& /*oldName*/, const QString /*newName*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::renameOptionGroup( const QString& oldName, const QString newName )" );
}

QMap<QString,int> SQLDB::SQLDB::classify( const ImageSearchInfo& info, const QString& category )
{
    // PENDING(blackie) count member maps too.

    QStringList matches = filesMatchingQuery( info );
    QMap<QString,int> result;

    for( QStringList::ConstIterator it = matches.begin(); it != matches.end(); ++it ) {
        QSqlQuery categoryQuery;
        QString queryString = QString::fromLatin1("SELECT value from imagecategoryinfo WHERE filename=\"%1\" and category=\"%2\"" )
                              .arg( *it ).arg( category );
        if ( !categoryQuery.exec( queryString ) )
            showError( categoryQuery.lastError(), queryString );

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

QStringList SQLDB::SQLDB::imageList( bool withRelativePath )
{
    QString queryString = QString::fromLatin1( "SELECT fileName FROM sortorder" );
    QSqlQuery query;
    if ( !query.exec( queryString ) )
        showError( query.lastError(), queryString );
    QStringList result;
    while ( query.next() ) {
        if ( withRelativePath )
            result <<  query.value(0).toString();
        else
            result << Options::instance()->imageDirectory() + query.value(0).toString();
    }
    return result;
}


QStringList SQLDB::SQLDB::images()
{
    return imageList( false );
}

void SQLDB::SQLDB::addImages( const ImageInfoList& images )
{
    int idx = totalCount();
    for( ImageInfoListIterator it( images ); *it; ++it ) {
        qDebug( "Inserting %s", (*it)->fileName().latin1());
        ImageInfo* info = *it;
        QString queryString = QString::fromLatin1( "INSERT INTO imageinfo set " );
        queryString += QString::fromLatin1( "width = %1, " ).arg( info->size().width() );
        queryString += QString::fromLatin1( "height = %1, " ).arg( info->size().height() );
        queryString += QString::fromLatin1( "md5sum = \"%1\", " ).arg( info->MD5Sum() );
        queryString += QString::fromLatin1( "fileName = \"%1\", " ).arg( info->fileName( true ) );
        queryString += QString::fromLatin1( "label = \"%1\", " ).arg( info->label() );
        queryString += QString::fromLatin1( "angle = %1, " ).arg( info->angle() );
        queryString += QString::fromLatin1( "description = \"%1\", " ).arg( info->description() );
        queryString += QString::fromLatin1( "yearFrom = %1, " ).arg( info->startDate().year() );
        queryString += QString::fromLatin1( "monthFrom = %1, " ).arg( info->startDate().month() );
        queryString += QString::fromLatin1( "dayFrom = %1, " ).arg( info->startDate().day() );
        queryString += QString::fromLatin1( "hour = %1, " ).arg( info->startDate().hour() );
        queryString += QString::fromLatin1( "minute = %1, " ).arg( info->startDate().minute() );
        queryString += QString::fromLatin1( "second = %1, " ).arg( info->startDate().second() );
        queryString += QString::fromLatin1( "yearTo = %1, " ).arg( info->endDate().year() );
        queryString += QString::fromLatin1( "monthTo = %1, " ).arg( info->endDate().month() );
        queryString += QString::fromLatin1( "dayTo = %1 " ).arg( info->endDate().day() );
        QSqlQuery query;
        if ( !query.exec( queryString ) )
            showError( query.lastError(), queryString );

        queryString = QString::fromLatin1( "INSERT INTO sortorder SET idx=%1, fileName=\"%2\"" ).arg(idx++).arg(info->fileName( true ) );
        if ( !query.exec( queryString ) )
            showError( query.lastError(), queryString );
    }
    emit totalChanged( totalCount() );
}

void SQLDB::SQLDB::addToBlockList( const QStringList& /*list*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::addToBlockList( const QStringList& list )" );
}

bool SQLDB::SQLDB::isBlocking( const QString& /*fileName*/ )
{
    qDebug("NYI: bool SQLDB::SQLDB::isBlocking( const QString& fileName )" );
    return false;
}

void SQLDB::SQLDB::deleteList( const QStringList& list )
{
    QStringList sortOrder = imageList( true );
    QSqlQuery query;
    QString queryString;
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        QString fileName = Util::stripImageDirectory( *it );

        queryString = QString::fromLatin1( "DELETE FROM imageinfo where fileName=\"%1\"" ).arg( fileName );
        if ( !query.exec( queryString ) )
            showError( query.lastError(), queryString );

        queryString = QString::fromLatin1( "DELETE FROM imagecategoryinfo where fileName=\"%1\"" ).arg( fileName );
        if ( !query.exec( queryString ) )
            showError( query.lastError(), queryString );

        sortOrder.remove( fileName );
    }

    queryString = QString::fromLatin1( "DELETE FROM sortorder" );
    if ( !query.exec( queryString ) )
        showError( query.lastError(), queryString );

    int idx = 0;
    for( QStringList::Iterator it = sortOrder.begin(); it != sortOrder.end(); ++it ) {
        queryString = QString::fromLatin1( "INSERT INTO sortorder SET idx=%1, fileName=\"%2\"" ).arg(idx++).arg( *it );
        if ( !query.exec( queryString ) )
            showError( query.lastError(), queryString );
    }

    qDebug("Images: %s", images().join( QString::fromLatin1( ", " ) ).latin1() );
}

ImageInfo* SQLDB::SQLDB::info( const QString& fileName ) const
{
    QString relativeFileName = Util::stripImageDirectory( fileName );

    static QMap<QString,ImageInfo*> map;
    if ( map.contains( relativeFileName ) )
        return map[relativeFileName];

    QSqlQuery query;
    QString queryString = QString::fromLatin1( "SELECT %1 FROM imageinfo where fileName=\"%2\"" ).arg( imageInfoAttributes ).arg( relativeFileName );
    if ( !query.exec( queryString ) )
        showError( query.lastError(), queryString );

    Q_ASSERT( query.numRowsAffected() == 1 );
    if ( query.numRowsAffected() != 1 )
        qWarning( "Internal Error: Didn't find %s in Database", fileName.latin1() );

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
    ImageInfo* info = new ImageInfo( relativeFileName, label, description, ImageDate( dayFrom, monthFrom, yearFrom, hour, minute, second ),
                                     ImageDate( dayTo, monthTo, yearTo ), angle, md5sum, QSize( width, heigh ) );


    queryString = QString::fromLatin1( "SELECT category, value FROM imagecategoryinfo WHERE fileName=\"%1\"" ).arg( relativeFileName );
    if ( !query.exec( queryString ) )
        showError( query.lastError(), queryString );

    while ( query.next() ) {
        info->addOption( query.value(0).toString(), query.value(1).toString() );
    }

    map.insert( relativeFileName, info );
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

