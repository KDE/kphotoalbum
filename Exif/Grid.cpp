#include "Grid.h"
#include "Info.h"
#include <QResizeEvent>
#include <QPainter>
#include <QTimer>

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
