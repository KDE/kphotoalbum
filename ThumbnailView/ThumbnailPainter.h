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

private:
    QTimer* _repaintTimer;
    QMutex _pendingRepaintLock;
    IdSet _pendingRepaint;
};

}

#endif /* THUMBNAILPAINTER_H */

