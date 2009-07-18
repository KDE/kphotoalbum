#ifndef THUMBNAILFACADE_H
#define THUMBNAILFACADE_H
#include "ThumbnailFactory.h"
#include "ThumbnailWidget.h"

class ThumbnailToolTip;
namespace ThumbnailView
{
class ThumbnailModel;
class CellGeometry;
class ThumbnailPainter;

class ThumbnailFacade :public QObject, public ThumbnailFactory
{
    Q_OBJECT
public:
    static ThumbnailFacade* instance();
    ThumbnailFacade();
    QWidget* gui();
    void setCurrentItem( const DB::ResultId& id );
    void reload( bool flushCache, bool clearSelection=true );
    DB::Result selection(bool keepSortOrderOfDatabase=false) const;
    DB::Result imageList(Order) const;
    DB::ResultId mediaIdUnderCursor() const;
    DB::ResultId currentItem() const;
    void setImageList(const DB::Result& list);
    void setSortDirection( SortDirection );

public slots:
    void gotoDate( const DB::ImageDate& date, bool includeRanges );
    void selectAll();
    void showToolTipsOnImages( bool b );
    void repaintScreen();
    void toggleStackExpansion(const DB::ResultId& id);
    void collapseAllStacks();
    void expandAllStacks();
    void updateDisplayModel();
    void changeSingleSelection(const DB::ResultId& id);
    void slotRecreateThumbnail();

signals:
    void showImage( const DB::ResultId& id );
    void showSelection();
    void fileNameUnderCursorChanged( const QString& fileName );
    void currentDateChanged( const QDateTime& );
    void selectionChanged();
    void collapseAllStacksEnabled(bool enabled);
    void expandAllStacksEnabled(bool enabled);

private:
    OVERRIDE ThumbnailModel* model();
    OVERRIDE CellGeometry* cellGeometry();
    OVERRIDE ThumbnailWidget* widget();
    OVERRIDE ThumbnailPainter* painter();
    OVERRIDE ThumbnailCache* cache();

private:
    static ThumbnailFacade* _instance;
    CellGeometry* _cellGeometry;
    ThumbnailModel* _model;
    ThumbnailCache* _thumbnailCache;
    ThumbnailWidget* _widget;
    ThumbnailPainter* _painter;
    ThumbnailToolTip* _toolTip;
};
}

#endif /* THUMBNAILFACADE_H */

