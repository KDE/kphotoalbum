/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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
#include "ThumbnailPainter.h"
#include <QColor>
#include "Cell.h"
#include "ThumbnailCache.h"
#include <QTimer>
#include "enums.h"
#include "ThumbnailFactory.h"
#include "ThumbnailModel.h"
#include "CellGeometry.h"
#include "DB/ImageDB.h"
#include <Q3PointArray>
#include "ImageManager/Manager.h"
#include "ThumbnailRequest.h"
#include "Settings/SettingsData.h"
#include "ThumbnailWidget.h"
#include <QPainter>
#include <QPixmap>
#include "Utilities/Set.h"
using Utilities::StringSet;

QColor contrastColor( const QColor& col )
{
    if ( col.red() < 127 && col.green() < 127 && col.blue() < 127 )
        return Qt::white;
    else
        return Qt::black;
}

ThumbnailView::ThumbnailPainter::ThumbnailPainter( ThumbnailFactory* factory )
    : ThumbnailComponent( factory )
{
    _repaintTimer = new QTimer( this );
    _repaintTimer->setSingleShot(true);
    connect( _repaintTimer, SIGNAL( timeout() ), this, SLOT( slotRepaint() ) );
}

void ThumbnailView::ThumbnailPainter::paintCell( QPainter * painter, int row, int col )
{
    paintCellBackground( painter, row, col );
    if ( !widget()->isGridResizing() ) {
        paintCellPixmap( painter, row, col );
        paintCellText( painter, row, col );
    }
}

static DB::StackID getStackId(const DB::ResultId& id)
{
    return id.fetchInfo()->stackId();
}


/**
 * Paint the pixmap in the cell (row,col)
 */
void ThumbnailView::ThumbnailPainter::paintCellPixmap( QPainter* painter, int row, int col )
{
    DB::ResultId mediaId = model()->imageAt( row, col );
    if (mediaId.isNull())
        return;

    QPixmap pixmap;
    if (cache()->find(mediaId, &pixmap)) {
        const QRect rect = cellGeometryInfo()->iconGeometry( row, col );
        Q_ASSERT( !rect.isNull() );

        painter->drawPixmap( rect, pixmap );

        // Paint transparent pixels over the widget for selection.
        if ( model()->isSelected(model()->imageAt(row, col)) )
            painter->fillRect( rect, QColor(58,98,134, 127) );

        paintBoundingRect( painter, row, col );
        paintDropIndicator( painter, row, col );
        paintStackedIndicator(painter, rect, mediaId);
    }
    else
        requestThumbnail( mediaId );
}

/**
 * Draw the title under the thumbnail
 */
void ThumbnailView::ThumbnailPainter::paintCellText( QPainter* painter, int row, int col )
{
    DB::ResultId mediaId = model()->imageAt( row, col );
    if ( mediaId.isNull() )
        return;

    QString title = thumbnailText( mediaId );
    QRect rect = cellTextGeometry( row, col );
    painter->setPen( contrastColor( Settings::SettingsData::instance()->backgroundColor() ) );

    //Qt::TextWordWrap just in case, if the text's width is wider than the cell's width
    painter->drawText( rect, Qt::AlignCenter | Qt::TextWordWrap, title );
}

/**
 * Paint the cell back ground, and the outline
 */
void ThumbnailView::ThumbnailPainter::paintCellBackground( QPainter* p, int row, int col )
{
    QRect rect = widget()->cellRect();
    p->fillRect( rect, QColor(Settings::SettingsData::instance()->backgroundColor()) );

    if (widget()->isGridResizing()
        || Settings::SettingsData::instance()->thumbnailDisplayGrid()) {
        p->setPen( contrastColor( Settings::SettingsData::instance()->backgroundColor() ) );
        // left of frame
        if ( col != 0 )
            p->drawLine( rect.left(), rect.top(), rect.left(), rect.bottom() );

        // bottom line
        if ( row != widget()->numRows() -1 ) {
            p->drawLine( rect.left(), rect.bottom() -1, rect.right(), rect.bottom()-1 );
        }
    }
}

QRect ThumbnailView::ThumbnailPainter::cellTextGeometry( int row, int col ) const
{
    if ( !Settings::SettingsData::instance()->displayLabels() && !Settings::SettingsData::instance()->displayCategories() )
        return QRect();

    DB::ResultId mediaId = model()->imageAt( row, col );
    if ( mediaId.isNull() ) // empty cell
        return QRect();

    int h = cellGeometryInfo()->textHeight( QFontMetrics( widget()->font() ).height(), false );

    QRect iconRect = cellGeometryInfo()->iconGeometry( row, col );
    QRect cellRect = const_cast<ThumbnailWidget*>(widget())->cellGeometry( row, col );

    return QRect( 1, cellRect.height() -h -1, cellRect.width()-2, h );
}

/**
 * Returns the text under the thumbnails
 */
QString ThumbnailView::ThumbnailPainter::thumbnailText( const DB::ResultId& mediaId ) const
{
    QString text;

    const QSize cellSize = cellGeometryInfo()->cellSize();
    int thumbnailHeight = cellSize.height() - 2 * Settings::SettingsData::instance()->thumbnailSpace();
    int thumbnailWidth = cellSize.width(); // no substracting here
    int maxCharacters = thumbnailHeight / QFontMetrics( widget()->font() ).maxWidth() * 2;

    if ( Settings::SettingsData::instance()->displayLabels()) {
        QString line = mediaId.fetchInfo()->label();
        if ( QFontMetrics( widget()->font() ).width( line ) > thumbnailWidth ) {
            line = line.left( maxCharacters );
            line += QString::fromLatin1( " ..." );
        }
        text += line + QString::fromLatin1("\n");
    }

    if ( Settings::SettingsData::instance()->displayCategories()) {
        QStringList grps = mediaId.fetchInfo()->availableCategories();
        for( QStringList::const_iterator it = grps.constBegin(); it != grps.constEnd(); ++it ) {
            QString category = *it;
            if ( category != QString::fromLatin1( "Folder" ) && category != QString::fromLatin1( "Media Type" ) ) {
                StringSet items = mediaId.fetchInfo()->itemsOfCategory( category );
                if (!items.empty()) {
                    QString line;
                    bool first = true;
                    for( StringSet::const_iterator it2 = items.begin(); it2 != items.end(); ++it2 ) {
                        QString item = *it2;
                        if ( first )
                            first = false;
                        else
                            line += QString::fromLatin1( ", " );
                        line += item;
                    }
                    if ( QFontMetrics( widget()->font() ).width( line ) > thumbnailWidth ) {
                        line = line.left( maxCharacters );
                        line += QString::fromLatin1( " ..." );
                    }
                    text += line + QString::fromLatin1( "\n" );
                }
            }
        }
    }

    if(text.isEmpty())
        text = QString::fromLatin1( "" );

    return text.trimmed();
}

void ThumbnailView::ThumbnailPainter::paintStackedIndicator( QPainter* painter, const QRect &rect, const DB::ResultId& mediaId)
{
    DB::ImageInfoPtr imageInfo = mediaId.fetchInfo();
    if (!imageInfo || !imageInfo->isStacked())
        return;

    const DB::StackID stackId  = imageInfo->stackId();
    bool isFirst = true;
    bool isLast = true;

    // A bit ugly: determine where we are within the stack.
    if (model()->isItemInExpandedStack(stackId)) {
        int prev = model()->indexOf(mediaId) - 1;
        int next = model()->indexOf(mediaId) + 1;
        isFirst = (prev < 0) || getStackId(model()->imageAt(prev)) != stackId;
        isLast  = (next >= model()->imageCount()) || getStackId(model()->imageAt(next)) != stackId;
    }

    const int thickness = 1;
    const int space = 0;
    const int corners = 8;
    const int w = rect.width();
    const int h = rect.height();
    int bottom_w, corner_h;
    bottom_w = corner_h = qMin(w / 2, h / 2);
    if (!isFirst)
        bottom_w = w;
    QPen pen;
    pen.setWidth(thickness);

    for (int c = 0; c < corners; ++c) {
        pen.setColor(c % 2 == 0 ? Qt::black : Qt::white);
        painter->setPen(pen);
        int step = (thickness + space) * c;
        int x = rect.x() + w - bottom_w - (thickness + space) * corners + step;
        int y = rect.y() + h - (thickness + space) * corners + step;
        painter->drawLine(x, y, rect.x() + w, y);
        if (isLast)
            painter->drawLine(x + bottom_w, y, x + bottom_w, y - corner_h);
    }
}


// ImageManager::ImageClient interface. Callback from the
// ImageManager when the image is loaded.
void ThumbnailView::ThumbnailPainter::pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int,
                                                   const QImage& image, const bool loadedOK)
{
    QPixmap pixmap( size );
    if ( loadedOK && !image.isNull() )
        pixmap = QPixmap::fromImage( image );
    else if ( !loadedOK )
        pixmap.fill( widget()->palette().color( QPalette::Dark));

    if ( !loadedOK || !DB::ImageInfo::imageOnDisk( fileName ) ) {
        QPainter p( &pixmap );
        p.setBrush( widget()->palette().base() );
        p.setWindow( 0, 0, 100, 100 );
        Q3PointArray pts;
        pts.setPoints( 3, 70,-1,  100,-1,  100,30 );
        p.drawConvexPolygon( pts );
    }

    DB::ResultId id = DB::ImageDB::instance()->ID_FOR_FILE( fileName );
    DB::ImageInfoPtr imageInfo = id.fetchInfo();
    // TODO(hzeller): figure out, why the size is set here. We do an implicit
    // write here to the database.
    if ( fullSize.isValid() ) {
        imageInfo->setSize( fullSize );
    }

    cache()->insert(id, pixmap);

    widget()->updateCell( id );
}

/**
 * \brief Returns whether the thumbnail for fileName is still needed.
 *
 * If the user scrolls down through the view, a back log of thumbnail
 * request may build up, which will slow down scrolling a lot. Therefore
 * the ImageManger has the capability to check whether a thumbnail
 * request is really needed, when it gets to load the given thumbnail.
 */
bool ThumbnailView::ThumbnailPainter::thumbnailStillNeeded( const QString& fileName ) const
{
    // PENDING(hzeller): this ID can come from the ThumbnailRequest
    DB::ResultId id = DB::ImageDB::instance()->ID_FOR_FILE( fileName );
    Cell pos = model()->positionForMediaId( id );
    return pos.row() >= widget()->firstVisibleRow( PartlyVisible ) && pos.row() <= widget()->lastVisibleRow( PartlyVisible );
}

void ThumbnailView::ThumbnailPainter::slotRepaint()
{
    // Create a local copy to make the _pendingRepaint accessible as soon as
    // possible.
    _pendingRepaintLock.lock();
    IdSet toRepaint(_pendingRepaint);
    _pendingRepaint.clear();
    _pendingRepaintLock.unlock();

    if ( (int) toRepaint.size() > widget()->numCols() * widget()->numRowsPerPage() / 2 )
        widget()->repaintScreen();
    else {
        for( IdSet::const_iterator it = toRepaint.begin(); it != toRepaint.end(); ++it ) {
            Cell cell = model()->positionForMediaId( *it );
            widget()->Q3GridView::repaintCell( cell.row(), cell.col() );
        }
    }
}

void ThumbnailView::ThumbnailPainter::repaint( const DB::ResultId& id )
{
    _pendingRepaintLock.lock();
    _pendingRepaint.insert( id );
    _pendingRepaintLock.unlock();

    // Do not trigger immediatly the repaint but wait a tiny bit - there might
    // be more coming which we then can paint in one shot.
    _repaintTimer->start( 10 );
}


/**
   This will paint the pixels around the thumbnail, which gives it a 3D
   effect, and also which indicates current image and selection state.
   The colors are fetched from looking at the Gwenview. I tried to see if I
   could figure out from the code how it was drawn, but failed at doing so.
*/
void ThumbnailView::ThumbnailPainter::paintBoundingRect( QPainter* painter, int row, int col  )
{
    QRect rect = cellGeometryInfo()->iconGeometry( row, col );

    rect.adjust(0,0,-1,-1);
    for ( int i = 0; i < 5; ++i ) {
        QColor color;
        if ( model()->isSelected(model()->imageAt(row, col)) ) {
            static QColor selectionColors[] = { QColor(58,98,134), QColor(96,161,221), QColor(93,165,228), QColor(132,186,237), QColor(62,95,128)};
            color = selectionColors[i];
        }

        else if ( widget()->mediaIdUnderCursor() == model()->imageAt( row, col ) ) {
            static QColor hoverColors[] = { QColor(46,99,152), QColor(121,136,151), QColor(121,136,151), QColor(126,145,163), QColor(109,126,142)};
            color = hoverColors[i];
        }

        else {
            color = Qt::black;
            color.setAlpha( (0.5 - 0.1*i) * 255 );
        }

        painter->setPen( color );
        rect.adjust(-1,-1,1,1 );
        painter->drawRect(rect);
    }
}

void ThumbnailView::ThumbnailPainter::paintDropIndicator( QPainter* painter, int row, int col )
{
    const QRect rect( 0, 0, widget()->cellWidth(), widget()->cellHeight() );
    const DB::ResultId mediaId = model()->imageAt( row, col );

    if ( model()->leftDropItem() == mediaId )
        painter->fillRect( rect.left(), rect.top(), 3, rect.height(), QBrush( Qt::red ) );

    else if ( model()->rightDropItem() == mediaId )
        painter->fillRect( rect.right() -2, rect.top(), 3, rect.height(), QBrush( Qt::red ) );
}

void ThumbnailView::ThumbnailPainter::requestThumbnail( const DB::ResultId& mediaId )
{
    DB::ImageInfoPtr imageInfo = mediaId.fetchInfo();
    const QSize cellSize = cellGeometryInfo()->cellSize();
    const int angle = imageInfo->angle();
    const int space = Settings::SettingsData::instance()->thumbnailSpace();
    ThumbnailRequest* request
        = new ThumbnailRequest(imageInfo->fileName(DB::AbsolutePath),
                               QSize( cellSize.width() - 2 * space,
                                      cellSize.height() - 2 * space),
                               angle, this );
    request->setPriority( ImageManager::ThumbnailVisible );
    ImageManager::Manager::instance()->load( request );
}

