#include "Exif/InfoDialog.h"
#include <klocale.h>
#include "Exif/Info.h"
#include <qlayout.h>
#include <qpainter.h>
#include <qtable.h>
#include <qevent.h>
#include <qtimer.h>
#include <qlabel.h>
#include "imagemanager.h"
#include "imagerequest.h"
#include "imagedb.h"

Exif::InfoDialog::InfoDialog( const QString& fileName, QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("EXIF Information"), Close, Close, parent, name, false )
{
    setWFlags( WDestructiveClose | getWFlags() );

    QWidget* top = plainPage();
    QVBoxLayout* vlay = new QVBoxLayout( top, 6 );

    // -------------------------------------------------- File name and pixmap
    QHBoxLayout* hlay = new QHBoxLayout( vlay, 6 );
    QLabel* label = new QLabel( fileName, top );
    QFont fnt = font();
    fnt.setPointSize( (int) (fnt.pointSize() * 1.2) );
    fnt.setWeight( QFont::Bold );
    label->setFont( fnt );
    label->setAlignment( AlignCenter );
    hlay->addWidget( label, 1 );

    _pix = new QLabel( top );
    hlay->addWidget( _pix );
    ImageManager::instance()->load( new ImageRequest( fileName, QSize( 128, 128 ), ImageDB::instance()->info(fileName)->angle(), this ) );

    // -------------------------------------------------- Exif Grid
    Exif::Grid* grid = new Exif::Grid( fileName, top );
    vlay->addWidget( grid );
    grid->setFocus();

    // -------------------------------------------------- Current Search
    hlay = new QHBoxLayout( vlay, 6 );
    label = new QLabel( i18n( "Current EXIF Label Search: "), top );
    hlay->addWidget( label );

    _searchLabel = new QLabel( top );
    _searchLabel->setPaletteForegroundColor( red );
    fnt = font();
    fnt.setWeight( QFont::Bold );
    _searchLabel->setFont( fnt );

    hlay->addWidget( _searchLabel );
    hlay->addStretch( 1 );

    connect( grid, SIGNAL( searchStringChanged( const QString& ) ), this, SLOT( updateSearchString( const QString& ) ) );
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
    :QGridView( parent, name )
{
    QMap<QString,QString> map = Exif::Info::instance()->infoForDialog( fileName );
    calculateMaxKeyWidth( map );
    setFocusPolicy( WheelFocus );
    setHScrollBarMode( AlwaysOff );

    Set<QString> groups = exifGroups( map );
    int index = 0;
    for( Set<QString>::Iterator groupIt = groups.begin(); groupIt != groups.end(); ++groupIt ) {
        if ( index %2 ) // We need to start next header in coloumn 0
            ++index;

        // Header for group.
        QStringList list = QStringList::split( QString::fromLatin1( "." ), *groupIt );
        _texts[index] = qMakePair( list[0], QString::null );
        list.pop_front();
        _texts[index+1] = qMakePair( QString::fromLatin1( "." ) + list.join( QString::fromLatin1( "." ) ), QString::null );
        _headers.insert( index );
        index += 2;

        // Items of group
        QMap<QString,QString> items = itemsForGroup( *groupIt, map );
        QStringList sorted = items.keys();
        sorted.sort();
        for( QStringList::Iterator exifIt = sorted.begin(); exifIt != sorted.end(); ++exifIt ) {
            _texts[index] = qMakePair ( exifNameNoGroup( *exifIt ), items[*exifIt] );
            ++index;
        }
    }

    setNumRows( _texts.count() / 2 + _texts.count() % 2);
    setNumCols( 2 );
    setCellWidth( 200 );
    setCellHeight( QFontMetrics( font() ).height() );
}

void Exif::Grid::paintCell( QPainter * p, int row, int col )
{
    int index = row * 2 + col;
    QColor background;
    bool isHeader = _headers.contains( 2* (index / 2) );
    if ( isHeader )
        background = lightGray;
    else
        background = (index % 4 == 0 || index % 4 == 3) ? white : QColor(226, 235, 250);

    p->fillRect( cellRect(), background );

    if ( isHeader ) {
        p->drawText( cellRect(), ((index % 2) ? AlignLeft : AlignRight ), _texts[index].first );
    }
    else {
        QString text = _texts[index].first;
        bool match = ( !_search.isEmpty() && text.contains( _search, false ) );
        QFont f(p->font());
        f.setWeight( match ? QFont::Bold : QFont::Normal );
        p->setFont( f );
        p->setPen( match ? red : black );
        p->drawText( cellRect(), AlignLeft, text);
        QRect rect = cellRect();
        rect.setX( _maxKeyWidth + 10 );
        p->drawText( rect, AlignLeft, _texts[index].second );
    }
}


QSize Exif::InfoDialog::sizeHint() const
{
    return QSize( 800, 400 );
}

Set<QString> Exif::Grid::exifGroups( const QMap<QString,QString>& exifInfo )
{
    Set<QString> result;
    for( QMap<QString,QString>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it ) {
        result.insert( groupName( it.key() ) );
    }
    return result;
}

QMap<QString,QString> Exif::Grid::itemsForGroup( const QString& group, const QMap<QString, QString>& exifInfo )
{
    QMap<QString,QString> result;
    for( QMap<QString,QString>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it ) {
        if ( groupName( it.key() ) == group )
            result.insert( it.key(), it.data() );
    }
    return result;
}

QString Exif::Grid::groupName( const QString& exifName )
{
    QStringList list = QStringList::split( QString::fromLatin1("."), exifName );
    list.pop_back();
    return list.join( QString::fromLatin1(".") );
}

QString Exif::Grid::exifNameNoGroup( const QString& fullName )
{
    return QStringList::split( QString::fromLatin1("."), fullName ).last();
}

void Exif::Grid::resizeEvent( QResizeEvent* )
{
    QTimer::singleShot( 0, this, SLOT( updateGrid() ) );
}

void Exif::Grid::updateGrid()
{
    setCellWidth( clipper()->width() / 2 );
}

void Exif::Grid::calculateMaxKeyWidth( const QMap<QString, QString>& exifInfo )
{
    QFont f = font();
    f.setWeight( QFont::Bold );
    QFontMetrics metrics( f );
    _maxKeyWidth = 0;
    for( QMap<QString,QString>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it ) {
        _maxKeyWidth = QMAX( _maxKeyWidth, metrics.width( exifNameNoGroup( it.key() ) ) );
    }
}

void Exif::Grid::keyPressEvent( QKeyEvent* e )
{
    switch ( e->key() ) {
    case Key_Down:
        scrollBy( 0, cellHeight() );
        return;
    case Key_Up:
        scrollBy( 0, -cellHeight() );
        return;
    case Key_PageDown:
        scrollBy( 0, (clipper()->height() - cellHeight() ));
        return;
    case Key_PageUp:
        scrollBy( 0, -(clipper()->height() - cellHeight()) );
        return;
    case Key_Backspace:
        _search.remove( _search.length()-1, 1 );
        emit searchStringChanged( _search );
        updateContents();
        return;
    case Key_Escape:
        QGridView::keyPressEvent( e ); // Propagate to close dialog.
        return;
    }

    if ( !e->text().isNull() ) {
        _search += e->text();
        emit searchStringChanged( _search );
        updateContents();
    }
}


void Exif::InfoDialog::pixmapLoaded( const QString& , const QSize& , const QSize& , int , const QImage& img, bool loadedOK )
{
    if ( loadedOK )
        _pix->setPixmap( img );
}
