#include "deletethumbnailsdialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qfile.h>
#include "options.h"
#include <qdir.h>

DeleteThumbnailsDialog::DeleteThumbnailsDialog( QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("Delete thumbnails" ), Cancel | User1, Cancel, parent, name )
{
    QWidget* top = plainPage();
    QVBoxLayout* layout = new QVBoxLayout( top, 10 );

    QLabel* label = new QLabel( i18n("Files about to be deleted: " ), top );
    layout->addWidget( label );

    QTextEdit* edit = new QTextEdit( top, "edit" );
    edit->setReadOnly( true );
    layout->addWidget( edit );

    setButtonText( User1, i18n("Delete") );
    connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotDeleteFiles() ) );
    connect( this, SIGNAL( user1Clicked() ), this, SLOT( accept() ) );
    resize( 600, 600 );

    findThumbnails( Options::instance()->imageDirectory() );
    edit->setText( _files.join( QString::fromLatin1("\n" ) ) );
}

void DeleteThumbnailsDialog::slotDeleteFiles()
{
    for( QStringList::Iterator it = _files.begin(); it != _files.end(); ++it ) {
        QFile::remove(*it);
    }
}

void DeleteThumbnailsDialog::findThumbnails( const QString& directory )
{
    QString tndir = directory + QString::fromLatin1( "/ThumbNails" );
    QDir dir( tndir );
    if ( dir.exists() ) {
        QStringList files = dir.entryList( QDir::Files );
        for( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
            _files << tndir + QString::fromLatin1("/") + *it;
        }
    }

    dir = QDir( directory );
    QStringList files = dir.entryList( QDir::Dirs );
    for( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
        if ( (*it) != QString::fromLatin1( "ThumbNails" ) &&
             (*it) != QString::fromLatin1( "." ) &&
             (*it) != QString::fromLatin1( ".." ) ) {
            findThumbnails( directory + QString::fromLatin1("/") + *it );
        }
    }
}

#include "deletethumbnailsdialog.moc"
