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
#ifndef THUMBNAILPAINTER_H
#define THUMBNAILPAINTER_H

#include "ThumbnailComponent.h"
#include "DB/ResultId.h"
#include <QMutex>
#include <QSet>
#include <QObject>
#include "ImageManager/ImageClient.h"
#include "enums.h"

class QTimer;
class QString;
class QRect;
class QPainter;

namespace ThumbnailView {

class ThumbnailWidget;
class CellGeometry;
class ThumbnailModel;
class ThumbnailFactory;

class ThumbnailPainter :public QObject, public ImageManager::ImageClient, private ThumbnailComponent
{
    Q_OBJECT

public:
    ThumbnailPainter( ThumbnailFactory* factory );
    void paintCell ( QPainter * p, int row, int col );
    OVERRIDE void pixmapLoaded( const QString&, const QSize& size, const QSize& fullSize, int, const QImage&, const bool loadedOK);
    bool thumbnailStillNeeded( const QString& fileName ) const;
    void repaint( const DB::ResultId& id );

private slots:
    void slotRepaint();

private:
    void paintCellPixmap( QPainter*, int row, int col );
    void paintCellText( QPainter*, int row, int col );
    void paintCellBackground( QPainter*, int row, int col );
    QRect cellTextGeometry( int row, int col ) const;
    QString thumbnailText( const DB::ResultId& mediaId ) const;
    void paintStackedIndicator( QPainter* painter, const QRect &rect, const DB::ResultId& mediaId);
    void paintBoundingRect( QPainter* painter, int row, int col );
    void paintDropIndicator( QPainter* painter, int row, int col );
    void requestThumbnail( const DB::ResultId& id );

private:
    QTimer* _repaintTimer;
    QMutex _pendingRepaintLock;
    IdSet _pendingRepaint;
};

}

#endif /* THUMBNAILPAINTER_H */

