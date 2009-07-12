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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <QComboBox>
#include "Exif/InfoDialog.h"
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
#include "DB/ResultId.h"

using Utilities::StringSet;

Exif::InfoDialog::InfoDialog( const DB::ResultId& id, QWidget* parent )
    :KDialog( parent )
{
    DB::ImageInfoPtr info = id.fetchInfo();
    QString fileName = info->fileName(DB::AbsolutePath);

    setWindowTitle( i18n("EXIF Information") );
    setButtons( Close );
    setWindowFlags( Qt::WDestructiveClose | windowFlags() );

    QWidget* top = new QWidget;
    setMainWidget( top );
    QVBoxLayout* vlay = new QVBoxLayout( top );

    // -------------------------------------------------- File name and pixmap
    QHBoxLayout* hlay = new QHBoxLayout;
    vlay->addLayout(hlay);
    QLabel* label = new QLabel( fileName, top );
    QFont fnt = font();
    fnt.setPointSize( (int) (fnt.pointSize() * 1.2) );
    fnt.setWeight( QFont::Bold );
    label->setFont( fnt );
    label->setAlignment( Qt::AlignCenter );
    hlay->addWidget( label, 1 );

    _pix = new QLabel( top );
    hlay->addWidget( _pix );
    ImageManager::ImageRequest* request = new ImageManager::ImageRequest( fileName, QSize( 128, 128 ), info->angle(), this );
    request->setPriority( ImageManager::Viewer );
    ImageManager::Manager::instance()->load( request );

    // -------------------------------------------------- Exif Grid
    Exif::Grid* grid = new Exif::Grid( fileName, top );
    vlay->addWidget( grid );
    grid->setFocus();

    // -------------------------------------------------- Current Search
    hlay = new QHBoxLayout;
    vlay->addLayout(hlay);
    label = new QLabel( i18n( "Current EXIF Label Search: "), top );
    hlay->addWidget( label );

    _searchLabel = new QLabel( top );
    QPalette pal = _searchLabel->palette();
    pal.setColor( QPalette::Foreground, Qt::red );
    _searchLabel->setPalette(pal);
    fnt = font();
    fnt.setWeight( QFont::Bold );
    _searchLabel->setFont( fnt );

    hlay->addWidget( _searchLabel );
    hlay->addStretch( 1 );

    QLabel* _iptcLabel = new QLabel( i18n("IPTC character set:"), top );
    _iptcCharset = new QComboBox( top );
    QStringList charsets;
    QList<QByteArray> charsetsBA = QTextCodec::availableCodecs();
    for (QList<QByteArray>::const_iterator it = charsetsBA.constBegin(); it != charsetsBA.constEnd(); ++it )
        charsets << QLatin1String(*it);
    _iptcCharset->insertItems( 0, charsets );
    _iptcCharset->setCurrentIndex( qMax( 0, QTextCodec::availableCodecs().indexOf( Settings::SettingsData::instance()->iptcCharset().toAscii() ) ) );
    hlay->addWidget( _iptcLabel );
    hlay->addWidget( _iptcCharset );

    connect( grid, SIGNAL( searchStringChanged( const QString& ) ), this, SLOT( updateSearchString( const QString& ) ) );
    connect( _iptcCharset, SIGNAL( activated( const QString& ) ), grid, SLOT( slotCharsetChange( const QString& ) ) );
    updateSearchString( QString::null );
}

void Exif::InfoDialog::updateSearchString( const QString& txt )
{
    if( txt.isEmpty() )
        _searchLabel->setText( i18n("<No Search>") );
    else
        _searchLabel->setText( txt );
}


Exif::Grid::Grid( const QString& fileName, QWidget* parent, const char* name )
    :Q3GridView( parent, name ), _fileName( fileName )
{
    setFocusPolicy( Qt::WheelFocus );
    setHScrollBarMode( AlwaysOff );

    slotCharsetChange( Settings::SettingsData::instance()->iptcCharset() );
}

void Exif::Grid::slotCharsetChange( const QString& charset )
{
    _texts.clear();
    _headers.clear();

    QMap<QString,QStringList> map = Exif::Info::instance()->infoForDialog( _fileName, charset );
    calculateMaxKeyWidth( map );

    StringSet groups = exifGroups( map );
    int index = 0;
    for( StringSet::const_iterator groupIt = groups.begin(); groupIt != groups.end(); ++groupIt ) {
        if ( index %2 ) // We need to start next header in coloumn 0
            ++index;

        // Header for group.
        QStringList list = (*groupIt).split(QString::fromLatin1( "." ));
        _texts[index] = qMakePair( list[0], QStringList() );
        list.pop_front();
        _texts[index+1] = qMakePair( QString::fromLatin1( "." ) + list.join( QString::fromLatin1( "." ) ), QStringList() );
        _headers.insert( index );
        index += 2;

        // Items of group
        QMap<QString,QStringList> items = itemsForGroup( *groupIt, map );
        QStringList sorted = items.keys();
        sorted.sort();
        for( QStringList::Iterator exifIt = sorted.begin(); exifIt != sorted.end(); ++exifIt ) {
            _texts[index] = qMakePair ( exifNameNoGroup( *exifIt ), items[*exifIt] );
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
    bool isHeader = _headers.contains( 2* (index / 2) );
    if ( isHeader )
        background = Qt::lightGray;
    else
        background = (index % 4 == 0 || index % 4 == 3) ? Qt::white : QColor(226, 235, 250);

    p->fillRect( cellRect(), background );

    if ( isHeader ) {
        p->drawText( cellRect(), ((index % 2) ? Qt::AlignLeft : Qt::AlignRight ), _texts[index].first );
    }
    else {
        QString text = _texts[index].first;
        bool match = ( !_search.isEmpty() && text.contains( _search, Qt::CaseInsensitive ) );
        QFont f(p->font());
        f.setWeight( match ? QFont::Bold : QFont::Normal );
        p->setFont( f );
        p->setPen( match ? Qt::red : Qt::black );
        p->drawText( cellRect(), Qt::AlignLeft, text);
        QRect rect = cellRect();
        rect.setX( _maxKeyWidth + 10 );
        p->drawText( rect, Qt::AlignLeft, _texts[index].second.join( QString::fromAscii(", ") ) );
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
    _maxKeyWidth = 0;
    for( QMap<QString,QStringList>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it ) {
        _maxKeyWidth = qMax( _maxKeyWidth, metrics.width( exifNameNoGroup( it.key() ) ) );
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
        _search.remove( _search.length()-1, 1 );
        emit searchStringChanged( _search );
        updateContents();
        return;
    case Qt::Key_Escape:
        Q3GridView::keyPressEvent( e ); // Propagate to close dialog.
        return;
    }

    if ( !e->text().isNull() ) {
        _search += e->text();
        emit searchStringChanged( _search );
        updateContents();
    }
}


void Exif::InfoDialog::pixmapLoaded( const QString& , const QSize& , const QSize& , int , const QImage& img, const bool loadedOK)
{
    if ( loadedOK )
        _pix->setPixmap( QPixmap::fromImage(img) );
}

#include "InfoDialog.moc"
