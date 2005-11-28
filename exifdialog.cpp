#include "exifdialog.h"
#include <klocale.h>
#include "exifinfo.h"
#include <qlayout.h>
#include <qpainter.h>
#include <qtable.h>
#include <qevent.h>
#include <qtimer.h>
#include <qlabel.h>

ExifDialog::ExifDialog( const QString& fileName, QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("EXIF Information"), Close, Close, parent, name )
{
    QWidget* top = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    ExifGrid* grid = new ExifGrid( fileName, top );
    lay1->addWidget( grid );
}

ExifGrid::ExifGrid( const QString& fileName, QWidget* parent, const char* name )
    :QGridView( parent, name )
{
    QMap<QString,QString> map = ExifInfo::instance()->infoForDialog( fileName );
    calculateMaxKeyWidth( map );


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

void ExifGrid::paintCell( QPainter * p, int row, int col )
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
        p->drawText( cellRect(), AlignLeft, _texts[index].first);
        QRect rect = cellRect();
        rect.setX( _maxKeyWidth + 10 );
        p->drawText( rect, AlignLeft, _texts[index].second );
    }
}


QSize ExifDialog::sizeHint() const
{
    return QSize( 800, 400 );
}

Set<QString> ExifGrid::exifGroups( const QMap<QString,QString>& exifInfo )
{
    Set<QString> result;
    for( QMap<QString,QString>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it ) {
        result.insert( groupName( it.key() ) );
    }
    return result;
}

QMap<QString,QString> ExifGrid::itemsForGroup( const QString& group, const QMap<QString, QString>& exifInfo )
{
    QMap<QString,QString> result;
    for( QMap<QString,QString>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it ) {
        if ( groupName( it.key() ) == group )
            result.insert( it.key(), it.data() );
    }
    return result;
}

QString ExifGrid::groupName( const QString& exifName )
{
    QStringList list = QStringList::split( QString::fromLatin1("."), exifName );
    list.pop_back();
    return list.join( QString::fromLatin1(".") );
}

QString ExifGrid::exifNameNoGroup( const QString& fullName )
{
    return QStringList::split( QString::fromLatin1("."), fullName ).last();
}

void ExifGrid::resizeEvent( QResizeEvent* )
{
    QTimer::singleShot( 0, this, SLOT( updateGrid() ) );
}

void ExifGrid::updateGrid()
{
    setCellWidth( clipper()->width() / 2 );
}

void ExifGrid::calculateMaxKeyWidth( const QMap<QString, QString>& exifInfo )
{
    _maxKeyWidth = 0;
    for( QMap<QString,QString>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it ) {
        _maxKeyWidth = QMAX( _maxKeyWidth, QFontMetrics( font() ).width( exifNameNoGroup( it.key() ) ) );
    }
}

#include "exifdialog.moc"
