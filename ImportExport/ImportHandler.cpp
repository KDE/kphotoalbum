#include "ImportHandler.h"
#include "Utilities/Util.h"

#include "KimFileReader.h"
#include "ImportSettings.h"
#include <QApplication>
#include <QFile>
#include <QProgressDialog>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kio/jobuidelegate.h>
#include "MainWindow/Window.h"
#include <kmessagebox.h>
#include "DB/ImageDB.h"
#include "Browser/BrowserWidget.h"
#include "DB/MD5Map.h"
#include "DB/Category.h"
#include "DB/CategoryCollection.h"
#include "Utilities/UniqFilenameMapper.h"
#include "kio/job.h"

using namespace ImportExport;

ImportExport::ImportHandler::ImportHandler()
    : m_fileMapper(NULL), m_finishedPressed(false), _progress(0), _reportUnreadableFiles( true )

{
}

ImportHandler::~ImportHandler() {
    delete m_fileMapper;
}

bool ImportExport::ImportHandler::exec( const ImportSettings& settings, KimFileReader* kimFileReader )
{
    m_settings = settings;
    m_kimFileReader = kimFileReader;
    m_finishedPressed = true;
    delete m_fileMapper;
    m_fileMapper = new Utilities::UniqFilenameMapper(m_settings.destination());
    bool ok;
    if ( m_settings.externalSource() ) {
        copyFromExternal();

        // If none of the images were to be copied, then we flushed the loop before we got started, in that case, don't start the loop.
        if ( _pendingCopies.count() > 0 )
            ok = m_eventLoop.exec();
        else
            ok = false;
    }
    else {
        ok = copyFilesFromZipFile();
        if ( ok )
            updateDB();
    }
    if ( _progress )
        delete _progress;

    return ok;
}

void ImportExport::ImportHandler::copyFromExternal()
{
    _pendingCopies = m_settings.selectedImages();
    _totalCopied = 0;
    _progress = new QProgressDialog( i18n("Copying Images"), i18n("&Cancel"), 0,2 * _pendingCopies.count(), MainWindow::Window::theMainWindow() );
    _progress->setValue( 0 );
    _progress->show();
    connect( _progress, SIGNAL( canceled() ), this, SLOT( stopCopyingImages() ) );
    copyNextFromExternal();

}

void ImportExport::ImportHandler::copyNextFromExternal()
{
    DB::ImageInfoPtr info = _pendingCopies[0];
    _pendingCopies.pop_front();

    if ( isImageAlreadyInDB( info ) ) {
        aCopyJobCompleted(0);
        return;
    }

    QString fileName = info->fileName( DB::RelativeToImageRoot );
    KUrl src1 = m_settings.kimFile();
    KUrl src2 = m_settings.baseURL();
    bool succeeded = false;
    QStringList tried;

    // First search for images next to the .kim file
    // Second search for images base on the image root as specified in the .kim file
    for ( int i = 0; i < 2; ++i ) {
        KUrl src = src1;
        if ( i == 1 )
            src = src2;

        src.setFileName( fileName );
        if ( KIO::NetAccess::exists( src, KIO::NetAccess::SourceSide, MainWindow::Window::theMainWindow() ) ) {
            KUrl dest;
            dest.setPath( m_fileMapper->uniqNameFor(fileName) );
            _job = KIO::file_copy( src, dest, -1, KIO::HideProgressInfo );
            connect( _job, SIGNAL( result( KJob* ) ), this, SLOT( aCopyJobCompleted( KJob* ) ) );
            succeeded = true;
            break;
        } else
            tried << src.prettyUrl();
    }

    if (!succeeded)
        aCopyFailed( tried );
}

bool ImportExport::ImportHandler::copyFilesFromZipFile()
{
    DB::ImageInfoList images = m_settings.selectedImages();

    _totalCopied = 0;
    _progress = new QProgressDialog( i18n("Copying Images"), i18n("&Cancel"), 0,2 * images.count(), MainWindow::Window::theMainWindow() );
    _progress->setValue( 0 );
    _progress->show();

    for( DB::ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it ) {
        if ( !isImageAlreadyInDB( *it ) ) {
            QString fileName = (*it)->fileName( DB::RelativeToImageRoot );
            QByteArray data = m_kimFileReader->loadImage( fileName );
            if ( data.isNull() )
                return false;
            QString newName = m_fileMapper->uniqNameFor(fileName);

            QFile out( newName );
            if ( !out.open( QIODevice::WriteOnly ) ) {
                KMessageBox::error( MainWindow::Window::theMainWindow(), i18n("Error when writing image %1", newName ) );
                return false;
            }
            out.write( data, data.size() );
            out.close();
        }

        qApp->processEvents();
        _progress->setValue( ++_totalCopied );
        if ( _progress->wasCanceled() ) {
            return false;
        }
    }
    return true;
}

void ImportExport::ImportHandler::updateDB()
{
    disconnect( _progress, SIGNAL( canceled() ), this, SLOT( stopCopyingImages() ) );
    _progress->setLabelText( i18n("Updating Database") );

    // Run though all images
    DB::ImageInfoList images = m_settings.selectedImages();
    for( DB::ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it ) {
        DB::ImageInfoPtr info = *it;

        if ( isImageAlreadyInDB( info ) )
            updateInfo( matchingInfoFromDB( info ), info );
        else
            addNewRecord( info );

        _progress->setValue( ++_totalCopied );
        if ( _progress->wasCanceled() )
            break;
    }

    Browser::BrowserWidget::instance()->home();
}

void ImportExport::ImportHandler::stopCopyingImages()
{
    _job->kill();
}

void ImportExport::ImportHandler::aCopyFailed( QStringList files )
{
    int result = _reportUnreadableFiles ?
                 KMessageBox::warningYesNoCancelList( _progress,
                                                      i18n("Cannot copy from any of the following locations:"),
                                                      files, QString::null, KStandardGuiItem::cont(), KGuiItem( i18n("Continue without Asking") )) : KMessageBox::Yes;

    switch (result) {
    case KMessageBox::Cancel:
        // This might be late -- if we managed to copy some files, we will
        // just throw away any changes to the DB, but some new image files
        // might be in the image directory...
        m_eventLoop.exit(false);
        break;

    case KMessageBox::No:
        _reportUnreadableFiles = false;
        // fall through
    default:
        aCopyJobCompleted( 0 );
    }
}

void ImportExport::ImportHandler::aCopyJobCompleted( KJob* job )
{
    if ( job && job->error() ) {
        job->uiDelegate()->showErrorMessage();
        m_eventLoop.exit(false);
    }
    else if ( _pendingCopies.count() == 0 ) {
        updateDB();
        m_eventLoop.exit(true);
    }
    else if ( _progress->wasCanceled() ) {
        m_eventLoop.exit(false);
    }
    else {
        _progress->setValue( ++_totalCopied );
        copyNextFromExternal();
    }
}

bool ImportExport::ImportHandler::isImageAlreadyInDB( const DB::ImageInfoPtr& info )
{
    return DB::ImageDB::instance()->md5Map()->contains(info->MD5Sum());
}

DB::ImageInfoPtr ImportExport::ImportHandler::matchingInfoFromDB( const DB::ImageInfoPtr& info )
{
    const QString& name = DB::ImageDB::instance()->md5Map()->lookup(info->MD5Sum());
    return DB::ImageDB::instance()->info( name, DB::RelativeToImageRoot );
}

void ImportExport::ImportHandler::updateInfo( DB::ImageInfoPtr dbInfo, DB::ImageInfoPtr newInfo )
{
    if ( dbInfo->label() != newInfo->label() && m_settings.importAction(QString::fromLatin1("*Label*")) == ImportSettings::Replace )
        dbInfo->setLabel( newInfo->label() );

    if ( dbInfo->description().simplified() != newInfo->description().simplified() ) {
        if ( m_settings.importAction(QString::fromLatin1("*Description*")) == ImportSettings::Replace )
            dbInfo->setDescription( newInfo->description() );
        else if ( m_settings.importAction(QString::fromLatin1("*Description*")) == ImportSettings::Merge )
            dbInfo->setDescription( dbInfo->description() + QString::fromLatin1("<br/><br/>") + newInfo->description() );
    }


    if (dbInfo->angle() != newInfo->angle() && m_settings.importAction(QString::fromLatin1("*Orientation*")) == ImportSettings::Replace )
        dbInfo->setAngle( newInfo->angle() );

    if (dbInfo->date() != newInfo->date() && m_settings.importAction(QString::fromLatin1("*Date*")) == ImportSettings::Replace )
        dbInfo->setDate( newInfo->date() );


    updateCategories( newInfo, dbInfo, false );
}

void ImportExport::ImportHandler::addNewRecord( DB::ImageInfoPtr info )
{
    QString importName = Utilities::stripImageDirectory(m_fileMapper->uniqNameFor(info->fileName(DB::RelativeToImageRoot)));

    DB::ImageInfoPtr updateInfo(new DB::ImageInfo(importName, DB::Image, false ));
    updateInfo->setLabel( info->label() );
    updateInfo->setDescription( info->description() );
    updateInfo->setDate( info->date() );
    updateInfo->setAngle( info->angle() );
    updateInfo->setMD5Sum( Utilities::MD5Sum( updateInfo->fileName(DB::AbsolutePath) ) );


    DB::ImageInfoList list;
    list.append(updateInfo);
    DB::ImageDB::instance()->addImages( list );

    updateCategories( info, updateInfo, true );
}

void ImportExport::ImportHandler::updateCategories( DB::ImageInfoPtr XMLInfo, DB::ImageInfoPtr DBInfo, bool forceReplace )
{
    // Run though the categories
    const QList<CategoryMatchSetting> matches = m_settings.categoryMatchSetting();

    Q_FOREACH( const CategoryMatchSetting& match, matches ) {
        QString XMLCategoryName = match.XMLCategoryName();
        QString DBCategoryName = match.DBCategoryName();
        ImportSettings::ImportAction action = m_settings.importAction(DBCategoryName);

        const Utilities::StringSet items = XMLInfo->itemsOfCategory(XMLCategoryName);
        DB::CategoryPtr DBCategoryPtr =  DB::ImageDB::instance()->categoryCollection()->categoryForName( DBCategoryName );

        if ( !forceReplace && action == ImportSettings::Replace )
            DBInfo->setCategoryInfo( DBCategoryName, Utilities::StringSet() );

        if ( action == ImportSettings::Merge || action == ImportSettings::Replace || forceReplace ) {
            Q_FOREACH( const QString& item, items ) {
                if (match.XMLtoDB().contains( item ) ) {
                    DBInfo->addCategoryInfo( DBCategoryName, match.XMLtoDB()[item] );
                    DBCategoryPtr->addItem( match.XMLtoDB()[item] );
                }
            }
        }
    }

}
