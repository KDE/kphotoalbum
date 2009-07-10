#include "Icon3DEffects.h"
#include <QPalette>
#include <QCache>
#include <QPainter>

QPixmap Utilities::Icon3DEffects::addEffects( const QPalette& palette, const QPixmap& pixmap )
{
    QPixmap result;

    // Trust me, I did try to just do a "return pixmap;" here, but for very odd reasons, that resulted in
    // unitialized pixels in the case where we actually did the effect. Very very odd!
    if ( pixmap.hasAlpha() ) {
        result = pixmap;
    }
    else {
        result = emptyPixmapTemplate( palette, pixmap.size() );

        // draw pixmap
        QPainter painter( &result );
        QPainterPath path;
        QRect rect = pixmap.rect();
        rect.adjust(0,0,-2,-2);
        const int BORDER = borderSizeForRoundedPixmap( pixmap.size() );
        path.addRoundedRect(rect, BORDER, BORDER );

        painter.setClipPath( path );
        painter.drawPixmap( 0,0,pixmap );
    }

    return result;
}


uint qHash( const QSize& size )
{
    return size.width() + size.height();
}

/**
 * The thumbnails are displayed in a rounded rect with a drop
 * shadow. Profiling indicated that 57% of the overall cost of displaying a
 * page of thumbnails was spent on paining this rounded rect. Two of those
 * were for drop shaddow, so this code is an optimization to speed up this drawing.
 *
 * I did try to draw the drop shadow without clipping - simply as two
 * lines on the right and lower edge of the picture, but the missing shadow
 * in the corner looked really odd.
 */
QPixmap Utilities::Icon3DEffects::emptyPixmapTemplate( const QPalette& palette, const QSize& size )
{
    static int maxCost = 1024* 1024 * 2;
    static QCache<QSize,QPixmap> templates( maxCost );
    int cost = size.width()*size.height()*4;

    // If the cost is larger than maxCost, then it is never inserted into the case, which would result in a crash.
    if ( cost > maxCost )
        cost = maxCost-1;

    if ( !templates.contains(size) ) {
        QPixmap result(size);
        const int BORDER = borderSizeForRoundedPixmap( size );

        // Initialize with transparent pixels - this is for the very few pixels at the corner
        result.fill( QColor( 255,255,255, 0) );

        const QRect rect( QPoint(0,0), size );
        // outermost drop shadow
        {
            QPainter painter( &result );
            QPainterPath path;
            path.addRoundedRect(rect, BORDER,BORDER );
            painter.setClipPath( path );
            painter.fillRect( rect.translated(BORDER,BORDER), palette.color(QPalette::Shadow) );
        }

        // innermost drop shadow
        {
            QPainter painter( &result );
            QPainterPath path;
            QRect rect2 = rect;
            rect2.adjust(0,0,-1,-1);
            path.addRoundedRect(rect2, BORDER,BORDER );
            painter.setClipPath( path );
            painter.fillRect( rect.translated(BORDER,BORDER), palette.color(QPalette::Dark) );
        }
#ifdef KDAB_TEMPORARILY_REMOVED
        const QRect rect( QPoint(0,0), QSize(size.width()-1,size.height()-1) );
        // outermost drop shadow
        {
            QPainter painter( &result );
            QPainterPath path;
            painter.setPen( palette.color(QPalette::Shadow) );
            painter.drawRoundedRect( rect, BORDER,BORDER );
        }

        // innermost drop shadow
        {
            QPainter painter( &result );
            painter.setPen( palette.color(QPalette::Dark) );
            QPainterPath path;
            QRect rect2 = rect;
            rect2.adjust(0,0,-1,-1);
            painter.drawRoundedRect(rect2, BORDER,BORDER );
        }
#endif //KDAB_TEMPORARILY_REMOVED

        templates.insert( size, new QPixmap( result ), cost );
    }

    return *templates[size];
}

int Utilities::Icon3DEffects::borderSizeForRoundedPixmap( const QSize& size )
{
    return qMin( static_cast<int>(size.width() * 0.05), 20 );
}
