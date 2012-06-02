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
#include "Delegate.h"
#include "Utilities/Set.h"
#include <QTime>
#include <QDebug>
#include "Utilities/Util.h"
#include "Settings/SettingsData.h"
#include "ThumbnailWidget.h"
#include "CellGeometry.h"
#include <QPainter>
#include "ThumbnailModel.h"
#include <KLocale>
ThumbnailView::Delegate::Delegate(ThumbnailFactory* factory )
    :ThumbnailComponent( factory )
{

}

OVERRIDE void ThumbnailView::Delegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    paintCellBackground( painter, option.rect );
    if ( widget()->isGridResizing())
        return;

    if ( index.data( Qt::DecorationRole ).value<QPixmap>().isNull() )
        return;

    paintCellPixmap( painter, option, index );
    paintCellText( painter, option, index );
}

void ThumbnailView::Delegate::paintCellBackground( QPainter* painter, const QRect& rect ) const
{
    painter->fillRect( rect, QColor(Settings::SettingsData::instance()->backgroundColor()) );

    if (widget()->isGridResizing() || Settings::SettingsData::instance()->thumbnailDisplayGrid()) {
        painter->setPen( Utilities::contrastColor( Settings::SettingsData::instance()->backgroundColor() ) );
        // left and right of frame
        painter->drawLine( rect.right(), rect.top(), rect.right(), rect.bottom() );

        // bottom line
        painter->drawLine( rect.left(), rect.bottom(), rect.right(), rect.bottom() );
    }
}

void ThumbnailView::Delegate::paintCellPixmap( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    const QPixmap pixmap = index.data( Qt::DecorationRole ).value<QPixmap>();

    const QRect pixmapRect = cellGeometryInfo()->iconGeometry( pixmap ).translated(option.rect.topLeft());
    paintBoundingRect( painter, pixmapRect, index );
    painter->drawPixmap( pixmapRect, pixmap );
    paintVideoInfo(painter, pixmapRect, index );
    paintDropIndicator( painter, option.rect, index );
    paintStackedIndicator(painter, pixmapRect, index);

    // Paint transparent pixels over the widget for selection.
    if ( widget()->selectionModel()->isSelected( index ) )
        painter->fillRect( option.rect, QColor(58,98,134, 127) );
}

void ThumbnailView::Delegate::paintVideoInfo(QPainter *painter, const QRect& pixmapRect, const QModelIndex &index) const
{
    DB::ImageInfoPtr imageInfo = model()->imageAt(index.row()).info();
    if (!imageInfo || imageInfo->mediaType() != DB::Video )
        return;

    const QString text = videoLengthText(imageInfo);
    const QRect metricsRect = painter->fontMetrics().boundingRect(text);

    const int margin = 3;
    const QRect textRect = QRect(pixmapRect.right()-metricsRect.width()-margin,
                                 pixmapRect.bottom()-metricsRect.height()-margin,
                                 metricsRect.width(), metricsRect.height());
    const QRect backgroundRect =  textRect.adjusted(-margin,-margin, margin, margin);

    if ( backgroundRect.width() > pixmapRect.width()/2  ) {
        // Dont show the time if the box would fill more than half the thumbnail
        return;
    }

    painter->save();
    painter->fillRect(backgroundRect, QBrush( QColor(0,0,0,128)));
    painter->setPen(Qt::white);
    painter->drawText(textRect, text);
    painter->restore();
}

void ThumbnailView::Delegate::paintCellText( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    // Optimization based on result from KCacheGrind
    if ( !Settings::SettingsData::instance()->displayLabels() && !Settings::SettingsData::instance()->displayCategories() )
        return;

    DB::FileName fileName = model()->imageAt( index.row() );
    if ( fileName.isNull() )
        return;

    QString title = index.data( Qt::DisplayRole ).value<QString>();
    QRect rect = cellGeometryInfo()->cellTextGeometry();
    painter->setPen( Utilities::contrastColor( Settings::SettingsData::instance()->backgroundColor() ) );

    //Qt::TextWordWrap just in case, if the text's width is wider than the cell's width
    painter->drawText( rect.translated( option.rect.topLeft() ), Qt::AlignCenter | Qt::TextWordWrap, title );
}

QSize ThumbnailView::Delegate::sizeHint( const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/ ) const
{
    return cellGeometryInfo()->cellSize();
}

/**
   This will paint the pixels around the thumbnail, which gives it a 3D
   effect, and also which indicates current image and selection state.
   The colors are fetched from looking at the Gwenview. I tried to see if I
   could figure out from the code how it was drawn, but failed at doing so.
*/
void ThumbnailView::Delegate::paintBoundingRect( QPainter* painter, const QRect& pixmapRect, const QModelIndex& index ) const
{
    QRect rect = pixmapRect;
    rect.adjust(-5,-5,4,4);
    for ( int i = 4; i >= 0; --i ) {
        QColor color;
        if ( widget()->selectionModel()->isSelected( index ) ) {
            static QColor selectionColors[] = { QColor(58,98,134), QColor(96,161,221), QColor(93,165,228), QColor(132,186,237), QColor(62,95,128)};
            color = selectionColors[i];
        }

#if 0
        // This code doesn't work very well with the QListView, for some odd reason, it often leaves a highlighted thumbnail behind
        //  9 Aug. 2010 11:33 -- Jesper K. Pedersen

        else if ( widget()->indexUnderCursor() == index ) {
            static QColor hoverColors[] = { QColor(46,99,152), QColor(121,136,151), QColor(121,136,151), QColor(126,145,163), QColor(109,126,142)};
            color = hoverColors[i];
        }
#endif

        else {
            // Originally I just painted the outline using drawRect, but that turned out to be a huge bottleneck.
            // The code was therefore converted to fillRect, which was much faster.
            // This code was complicted from that, as I previously drew the
            // rects from insite out, but with fillRect that doesn't work,
            // and I thefore had to rewrite the code to draw the rects from
            // outside in.
            // I now had to calculate the destination color myself, rather
            // than rely on drawing with a transparent color on top of the
            // background.
            // 12 Aug. 2010 17:38 -- Jesper K. Pedersen
            const QColor foreground = Qt::black;
            const QColor backround = QColor(Settings::SettingsData::instance()->backgroundColor());

            double alpha = (0.5 - 0.1*i);
            double inverseAlpha = 1 - alpha;

            color = QColor( foreground.red() * alpha + backround.red() * inverseAlpha,
                            foreground.green() * alpha + backround.green() * inverseAlpha,
                            foreground.blue() * alpha + backround.blue() * inverseAlpha );
        }

        QPen pen( color );
        painter->setPen(pen);
        painter->fillRect(rect, QBrush( color ) );
        rect.adjust(1,1,-1,-1 );
    }
}

static DB::StackID getStackId(const DB::FileName& fileName)
{
    return fileName.info()->stackId();
}

void ThumbnailView::Delegate::paintStackedIndicator( QPainter* painter, const QRect &pixmapRect, const QModelIndex& index ) const
{
    DB::ImageInfoPtr imageInfo = model()->imageAt(index.row()).info();
    if (!imageInfo || !imageInfo->isStacked())
        return;

    const QRect cellRect = widget()->visualRect( index );

    // Calculate the three points for the bottom most/right most lines
    int leftX = cellRect.left();
    int rightX = cellRect.right() + 5; // 5 for the 3D effect

    if ( isFirst( index.row() ) )
        leftX = pixmapRect.left() + pixmapRect.width()/2;

    if ( isLast( index.row() ) )
        rightX = pixmapRect.right();

    QPoint bottomLeftPoint( leftX, pixmapRect.bottom() );
    QPoint bottomRightPoint( rightX, pixmapRect.bottom() );
    QPoint topPoint = isLast( index.row() ) ? QPoint( rightX, pixmapRect.top() + pixmapRect.height()/2 ) : QPoint();

    // Paint the lines.
    painter->save();
    for ( int i=0; i < 8; ++i ) {
        painter->setPen( QPen(i % 2 == 0 ? Qt::black : Qt::white) );

        painter->drawLine(bottomLeftPoint,bottomRightPoint);
        if ( topPoint != QPoint() ) {
            painter->drawLine( bottomRightPoint, topPoint );
            topPoint -= QPoint(1,1);
        }

        bottomLeftPoint -= QPoint( isFirst( index.row()) ? 1 : 0, 1 );
        bottomRightPoint -= QPoint( isLast( index.row()) ? 1 : 0, 1);
    }
    painter->restore();
}

bool ThumbnailView::Delegate::isFirst( int row ) const
{
    const DB::StackID curId = getStackId(model()->imageAt(row));

    return
            !model()->isItemInExpandedStack(curId) ||
            row == 0 ||
            getStackId(model()->imageAt(row-1)) != curId;
}

bool ThumbnailView::Delegate::isLast( int row ) const
{
    const DB::StackID curId = getStackId(model()->imageAt(row));

    return
            !model()->isItemInExpandedStack(curId) ||
            row == model()->imageCount() -1 ||
            getStackId(model()->imageAt(row+1)) != curId;
}

QString ThumbnailView::Delegate::videoLengthText(const DB::ImageInfoPtr &imageInfo) const
{
    const int length = imageInfo->videoLength();
    if ( length < 0 )
        return i18n("video");

    const int hours = length/60/60;
    const int minutes = (length/60)%60;
    const int secs = length % 60;

    QString res;
    if (hours > 0)
        res = QString::number(hours) + QLatin1String(":");

    if (minutes < 10 && hours > 0)
        res += QLatin1String("0");

    res += QString::number(minutes);
    res += QLatin1String(":");

    if (secs < 10)
        res += QLatin1String("0");

    res += QString::number(secs);

    return res;
}


void ThumbnailView::Delegate::paintDropIndicator( QPainter* painter, const QRect& rect, const QModelIndex& index ) const
{
    const DB::FileName fileName = model()->imageAt( index.row() );

    if ( model()->leftDropItem() == fileName )
        painter->fillRect( rect.left(), rect.top(), 3, rect.height(), QBrush( Qt::red ) );

    else if ( model()->rightDropItem() == fileName )
        painter->fillRect( rect.right() -2, rect.top(), 3, rect.height(), QBrush( Qt::red ) );
}


