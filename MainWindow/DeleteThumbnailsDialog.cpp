/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kdeversion.h>
#include "DeleteThumbnailsDialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <q3textedit.h>
#include <qfile.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include "Settings/SettingsData.h"
#include <qdir.h>

using namespace MainWindow;

DeleteThumbnailsDialog::DeleteThumbnailsDialog( QWidget* parent, const char* name )
#ifdef TEMPORARILY_REMOVED
    :KDialog( Plain, i18n("Delete Thumbnails" ), Cancel | User1, Cancel, parent, name )
#endif
{
#ifdef TEMPORARILY_REMOVED
    QWidget* top = plainPage();
    Q3VBoxLayout* layout = new Q3VBoxLayout( top, 10 );

    QLabel* label = new QLabel( i18n("Files about to be deleted: " ), top );
    layout->addWidget( label );

    Q3TextEdit* edit = new Q3TextEdit( top, "edit" );
    edit->setReadOnly( true );
    layout->addWidget( edit );

#if !KDE_IS_VERSION( 3,3,0 )
    setButtonText( User1, i18n("&Delete") );
#else
    setButtonGuiItem( User1, KGuiItem(i18n("&Delete"),QString::fromLatin1("editdelete")) );
#endif

    connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotDeleteFiles() ) );
    connect( this, SIGNAL( user1Clicked() ), this, SLOT( accept() ) );
    resize( 600, 600 );

    findThumbnails( Settings::SettingsData::instance()->imageDirectory() );
    edit->setText( _files.join( QString::fromLatin1("\n" ) ) );
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
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

#include "DeleteThumbnailsDialog.moc"
