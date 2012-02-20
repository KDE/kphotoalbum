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
#include "Exif/InfoDialog.h"
#include <KComboBox>
#include <klocale.h>
#include "Exif/Info.h"
#include <qlayout.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qlabel.h>
#include <QTextCodec>
#include "ImageManager/Manager.h"
#include "ImageManager/ImageRequest.h"
#include "DB/ImageDB.h"
#include "Settings/SettingsData.h"
#include "DB/Id.h"

using Utilities::StringSet;

Exif::InfoDialog::InfoDialog( const DB::Id& id, QWidget* parent )
    :KDialog( parent )
{
    setWindowTitle( i18n("EXIF Information") );
    setButtons( Close );
    setWindowFlags( Qt::WDestructiveClose | windowFlags() );

    QWidget* top = new QWidget;
    setMainWidget( top );
    QVBoxLayout* vlay = new QVBoxLayout( top );

    // -------------------------------------------------- File name and pixmap
    QHBoxLayout* hlay = new QHBoxLayout;
    vlay->addLayout(hlay);
    m_fileNameLabel = new QLabel( top );
    QFont fnt = font();
    fnt.setPointSize( (int) (fnt.pointSize() * 1.2) );
    fnt.setWeight( QFont::Bold );
    m_fileNameLabel->setFont( fnt );
    m_fileNameLabel->setAlignment( Qt::AlignCenter );
    hlay->addWidget( m_fileNameLabel, 1 );

    m_pix = new QLabel( top );
    hlay->addWidget( m_pix );

    // -------------------------------------------------- Exif Grid
    m_grid = new Exif::Grid( top );
    vlay->addWidget( m_grid );

    // -------------------------------------------------- Current Search
    hlay = new QHBoxLayout;
    vlay->addLayout(hlay);
    m_fileNameLabel = new QLabel( i18n( "Current EXIF Label Search: "), top );
    hlay->addWidget( m_fileNameLabel );

    m_searchLabel = new QLabel( top );
    QPalette pal = m_searchLabel->palette();
    pal.setColor( QPalette::Foreground, Qt::red );
    m_searchLabel->setPalette(pal);
    fnt = font();
    fnt.setWeight( QFont::Bold );
    m_searchLabel->setFont( fnt );

    hlay->addWidget( m_searchLabel );
    hlay->addStretch( 1 );

    QLabel* iptcLabel = new QLabel( i18n("IPTC character set:"), top );
    m_iptcCharset = new KComboBox( top );
    QStringList charsets;
    QList<QByteArray> charsetsBA = QTextCodec::availableCodecs();
    for (QList<QByteArray>::const_iterator it = charsetsBA.constBegin(); it != charsetsBA.constEnd(); ++it )
        charsets << QLatin1String(*it);
    m_iptcCharset->insertItems( 0, charsets );
    m_iptcCharset->setCurrentIndex( qMax( 0, QTextCodec::availableCodecs().indexOf( Settings::SettingsData::instance()->iptcCharset().toAscii() ) ) );
    hlay->addWidget( iptcLabel );
    hlay->addWidget( m_iptcCharset );

    connect( m_grid, SIGNAL( searchStringChanged( const QString& ) ), this, SLOT( updateSearchString( const QString& ) ) );
    connect( m_iptcCharset, SIGNAL( activated( const QString& ) ), m_grid, SLOT( slotCharsetChange( const QString& ) ) );
    setImage(id);
    updateSearchString( QString() );
}

void Exif::InfoDialog::updateSearchString( const QString& txt )
{
    if( txt.isEmpty() )
        m_searchLabel->setText( i18n("<No Search>") );
    else
        m_searchLabel->setText( txt );
}


Exif::Grid::Grid( QWidget* parent, const char* name )
    :Q3GridView( parent, name )
{
    setFocusPolicy( Qt::WheelFocus );
    setHScrollBarMode( AlwaysOff );
}

void Exif::Grid::slotCharsetChange( const QString& charset )
{
    m_texts.clear();
    m_headers.clear();

    QMap<QString,QStringList> map = Exif::Info::instance()->infoForDialog( m_fileName, charset );
    calculateMaxKeyWidth( map );

    StringSet groups = exifGroups( map );
    int index = 0;
    for( StringSet::const_iterator groupIt = groups.begin(); groupIt != groups.end(); ++groupIt ) {
        if ( index %2 ) // We need to start next header in column 0
            ++index;

        // Header for group.
        QStringList list = (*groupIt).split(QString::fromLatin1( "." ));
        m_texts[index] = qMakePair( list[0], QStringList() );
        list.pop_front();
        m_texts[index+1] = qMakePair( QString::fromLatin1( "." ) + list.join( QString::fromLatin1( "." ) ), QStringList() );
        m_headers.insert( index );
        index += 2;

        // Items of group
        QMap<QString,QStringList> items = itemsForGroup( *groupIt, map );
        QStringList sorted = items.keys();
        sorted.sort();
        for( QStringList::Iterator exifIt = sorted.begin(); exifIt != sorted.end(); ++exifIt ) {
            m_texts[index] = qMakePair ( exifNameNoGroup( *exifIt ), items[*exifIt] );
            ++index;
        }
    }

    setNumRows( index / 2 + index % 2 );
    setNumCols( 2 );
    setCellWidth( 200 );
    setCellHeight( QFontMetrics( font() ).height() );

    // without this, grid is only partially drawn
    QResizeEvent re( size(), size() );
    resizeEvent( &re );
}

void Exif::Grid::paintCell( QPainter * p, int row, int col )
{
    int index = row * 2 + col;
    QColor background;
    bool isHeader = m_headers.contains( 2* (index / 2) );
    if ( isHeader )
        background = Qt::lightGray;
    else
        background = (index % 4 == 0 || index % 4 == 3) ? Qt::white : QColor(226, 235, 250);

    p->fillRect( cellRect(), background );

    if ( isHeader ) {
        p->drawText( cellRect(), ((index % 2) ? Qt::AlignLeft : Qt::AlignRight ), m_texts[index].first );
    }
    else {
        QString text = m_texts[index].first;
        bool match = ( !m_search.isEmpty() && text.contains( m_search, Qt::CaseInsensitive ) );
        QFont f(p->font());
        f.setWeight( match ? QFont::Bold : QFont::Normal );
        p->setFont( f );
        p->setPen( match ? Qt::red : Qt::black );
        p->drawText( cellRect(), Qt::AlignLeft, text);
        QRect rect = cellRect();
        rect.setX( m_maxKeyWidth + 10 );
        p->drawText( rect, Qt::AlignLeft, m_texts[index].second.join( QString::fromAscii(", ") ) );
    }
}


QSize Exif::InfoDialog::sizeHint() const
{
    return QSize( 800, 400 );
}

StringSet Exif::Grid::exifGroups( const QMap<QString,QStringList>& exifInfo )
{
    StringSet result;
    for( QMap<QString,QStringList>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it ) {
        result.insert( groupName( it.key() ) );
    }
    return result;
}

QMap<QString,QStringList> Exif::Grid::itemsForGroup( const QString& group, const QMap<QString, QStringList>& exifInfo )
{
    QMap<QString,QStringList> result;
    for( QMap<QString,QStringList>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it ) {
        if ( groupName( it.key() ) == group )
            result.insert( it.key(), it.value() );
    }
    return result;
}

QString Exif::Grid::groupName( const QString& exifName )
{
    QStringList list = exifName.split(QString::fromLatin1("."));
    list.pop_back();
    return list.join( QString::fromLatin1(".") );
}

QString Exif::Grid::exifNameNoGroup( const QString& fullName )
{
    return fullName.split(QString::fromLatin1(".")).last();
}

void Exif::Grid::resizeEvent( QResizeEvent* )
{
    QTimer::singleShot( 0, this, SLOT( updateGrid() ) );
}

void Exif::Grid::updateGrid()
{
    setCellWidth( clipper()->width() / 2 );
}

void Exif::Grid::calculateMaxKeyWidth( const QMap<QString, QStringList>& exifInfo )
{
    QFont f = font();
    f.setWeight( QFont::Bold );
    QFontMetrics metrics( f );
    m_maxKeyWidth = 0;
    for( QMap<QString,QStringList>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it ) {
        m_maxKeyWidth = qMax( m_maxKeyWidth, metrics.width( exifNameNoGroup( it.key() ) ) );
    }
}

void Exif::Grid::keyPressEvent( QKeyEvent* e )
{
    switch ( e->key() ) {
    case Qt::Key_Down:
        scrollBy( 0, cellHeight() );
        return;
    case Qt::Key_Up:
        scrollBy( 0, -cellHeight() );
        return;
    case Qt::Key_PageDown:
        scrollBy( 0, (clipper()->height() - cellHeight() ));
        return;
    case Qt::Key_PageUp:
        scrollBy( 0, -(clipper()->height() - cellHeight()) );
        return;
    case Qt::Key_Backspace:
        m_search.remove( m_search.length()-1, 1 );
        emit searchStringChanged( m_search );
        updateContents();
        return;
    case Qt::Key_Escape:
        Q3GridView::keyPressEvent( e ); // Propagate to close dialog.
        return;
    }

    if ( !e->text().isEmpty() ) {
        m_search += e->text();
        emit searchStringChanged( m_search );
        updateContents();
    }
}


void Exif::InfoDialog::pixmapLoaded( const QString& , const QSize& , const QSize& , int , const QImage& img, const bool loadedOK)
{
    if ( loadedOK )
      m_pix->setPixmap( QPixmap::fromImage(img) );
}

void Exif::InfoDialog::setImage(const DB::Id &id)
{
    DB::ImageInfoPtr info = id.fetchInfo();
    if ( info.isNull() )
        return;
    QString fileName = info->fileName(DB::AbsolutePath);
    m_fileNameLabel->setText( fileName );
    m_grid->setFileName( fileName );

    ImageManager::ImageRequest* request = new ImageManager::ImageRequest( fileName, QSize( 128, 128 ), info->angle(), this );
    request->setPriority( ImageManager::Viewer );
    ImageManager::Manager::instance()->load( request );
}

void Exif::Grid::setFileName(const QString &fileName)
{
    m_fileName = fileName;
    slotCharsetChange( Settings::SettingsData::instance()->iptcCharset() );
}

void Exif::InfoDialog::enterEvent(QEvent *)
{
    m_grid->setFocus();
}

#include "InfoDialog.moc"
