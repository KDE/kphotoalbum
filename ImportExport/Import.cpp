/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "Import.h"
#include "ImportHandler.h"
#include <klocale.h>
#include "KimFileReader.h"
#include <kmessagebox.h>
#include <KTemporaryFile>
#include <kfiledialog.h>
#include "ImportDialog.h"
#include <kio/jobuidelegate.h>
#include "MainWindow/Window.h"
#include <kio/job.h>

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
    :m_tmp(nullptr)
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
    connect( job, SIGNAL(result(KJob*)), this, SLOT(downloadKimJobCompleted(KJob*)) );
    connect( job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(data(KIO::Job*,QByteArray)) );
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
    KimFileReader kimFileReader;
    if ( !kimFileReader.open( fileName ) ) {
        delete this;
        return;
    }

    bool ok = dialog.exec( &kimFileReader, m_kimFileUrl );

    if ( ok ) {
        ImportHandler handler;
        handler.exec(dialog.settings(), &kimFileReader);
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

// vi:expandtab:tabstop=4 shiftwidth=4:
