#include "Import.h"
#include <QBuffer>
#include <kmessagebox.h>
#include <QDebug>
#include <KTemporaryFile>
#include <kfiledialog.h>
#include "ImportDialog.h"
#include <kio/jobuidelegate.h>
#include <kio/jobclasses.h>
#include "MainWindow/Window.h"

using namespace ImportExport;

void Import::imageImport()
{
    KUrl url = KFileDialog::getOpenUrl( KUrl(), QString::fromLatin1( "*.kim|KPhotoAlbum Export Files" ) );
    if ( url.isEmpty() )
        return;

    imageImport( url );
    // This instance will delete itself when done.
}

void Import::imageImport( const KUrl& url )
{
    Import* import = new Import;
    import->m_kimFileUrl = url;
    if ( !url.isLocalFile() )
         import->downloadUrl(url);
    else
        import->exec(url.path());
    // This instance will delete itself when done.
}

ImportExport::Import::Import()
    :m_tmp(0)
{
}

void ImportExport::Import::downloadUrl( const KUrl& url )
{
    m_tmp = new KTemporaryFile;
    m_tmp->setSuffix(QString::fromLatin1(".kim"));
    if ( !m_tmp->open() ) {
        KMessageBox::error( MainWindow::Window::theMainWindow(), i18n("Unable to create temporary file") );
        delete this;
        return;
    }
    KIO::TransferJob* job = KIO::get( url );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( downloadKimJobCompleted( KJob* ) ) );
    connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ), this, SLOT( data( KIO::Job*, const QByteArray& ) ) );
}

void ImportExport::Import::downloadKimJobCompleted( KJob* job )
{
    if ( job->error() ) {
        job->uiDelegate()->showErrorMessage();
        delete this;
    }
    else {
        QString path = m_tmp->fileName();
        m_tmp->close();
        exec( path );
    }
}

void ImportExport::Import::exec(const QString& fileName )
{
    ImportDialog dialog(MainWindow::Window::theMainWindow());
    bool ok = dialog.exec( fileName, m_kimFileUrl );

    if ( ok ) {
        ImportHandler handler( &dialog );
        handler.exec();
    }

    delete this;
}

void ImportExport::Import::data( KIO::Job*, const QByteArray& data )
{
    m_tmp->write( data );
}

ImportExport::Import::~Import()
{
    delete m_tmp;
}

