#ifndef THUMBNAILVIEW_H
#define THUMBNAILVIEW_H

#include <qgridview.h>
#include "ImageManager/ImageClient.h"
#include "Utilities/Set.h"
#include "ThumbnailToolTip.h"
#include "GridResizeInteraction.h"
#include "SelectionInteraction.h"
#include "MouseTrackingInteraction.h"
#include "Cell.h"
#include <qvaluevector.h>
#include "DB/ImageDate.h"

class QTimer;
class ImageDateRange;
class QDateTime;
class QPixmapCache;

namespace ThumbnailView
{
enum SortDirection {NewestFirst, OldestFirst};

class ThumbnailWidget : public QGridView, public ImageManager::ImageClient {
    Q_OBJECT

    static const int SPACE = 3;

public:
    ThumbnailWidget( QWidget* parent, const char* name = 0 );
    void setImageList( const QStringList& list );

    virtual void paintCell ( QPainter * p, int row, int col );
    virtual void pixmapLoaded( const QString&, const QSize& size, const QSize& fullSize, int, const QImage&, bool loadedOK );
    bool thumbnailStillNeeded( const QString& fileName ) const;
    QStringList selection( bool keepSortOrderOfDatabase = false ) const;

    enum Order { ViewOrder, SortedOrder };
    QStringList imageList( Order ) const;
    void reload( bool flushCache );
    QString fileNameUnderCursor() const;
    QString currentItem() const;
    static ThumbnailWidget* theThumbnailView();
    void setCurrentItem( const QString& fileName );
    void setSortDirection( SortDirection );
    void repaintScreen();


public slots:
    void gotoDate( const DB::ImageDate& date, bool includeRanges );
    void selectAll();
    void showToolTipsOnImages( bool b );

signals:
    void showImage( const QString& fileName );
    void showSelection();
    void fileNameUnderCursorChanged( const QString& fileName );
    void currentDateChanged( const QDateTime& );
    void selectionChanged();

protected:
    // Painting
    void updateCell( const QString& fileName );
    void updateCell( int row, int col );
    void paintCellBackground( QPainter*, int row, int col );
    void paintCellPixmap( QPainter*, int row, int col );
    void paintCellText( QPainter*, int row, int col );
    virtual void viewportPaintEvent( QPaintEvent* );

    // Cell handling methods.
    QString fileNameInCell( int row, int col ) const;
    QString fileNameInCell( const Cell& cell ) const;
    QString fileNameAtCoordinate( const QPoint& coordinate, CoordinateSystem ) const;
    Cell positionForFileName( const QString& fileName ) const;
    Cell cellAtCoordinate( const QPoint& pos, CoordinateSystem ) const;

    enum VisibleState { FullyVisible, PartlyVisible };
    int firstVisibleRow( VisibleState ) const;
    int lastVisibleRow( VisibleState ) const;
    int numRowsPerPage() const;
    QRect iconGeometry( int row, int col ) const;
    QRect cellTextGeometry( int row, int col ) const;
    bool isFocusAtFirstCell() const;
    bool isFocusAtLastCell() const;
    Cell lastCell() const;

    // event handlers
    virtual void keyPressEvent( QKeyEvent* );
    virtual void showEvent( QShowEvent* );
    virtual void mousePressEvent( QMouseEvent* );
    virtual void mouseMoveEvent( QMouseEvent* );
    virtual void mouseReleaseEvent( QMouseEvent* );
    virtual void mouseDoubleClickEvent ( QMouseEvent* );
    virtual void resizeEvent( QResizeEvent* );
    void keyboardMoveEvent( QKeyEvent* );
    virtual void dimensionChange ( int oldNumRows, int oldNumCols );

    // Selection
    void selectAllCellsBetween( Cell pos1, Cell pos2, bool repaint = true );
    void selectCell( int row, int col, bool repaint = true );
    void selectCell( const Cell& );
    void clearSelection();
    void toggleSelection( const QString& fileName );
    void possibleEmitSelectionChanged();

    // Drag and drop
    virtual void contentsDragMoveEvent ( QDragMoveEvent * );
    virtual void contentsDragLeaveEvent ( QDragLeaveEvent * );
    virtual void contentsDropEvent ( QDropEvent * );
    void removeDropIndications();

    // Misc
    void updateGridSize();
    bool isMovementKey( int key );
    void selectItems( const Cell& start, const Cell& end );
    void ensureCellsSorted( Cell& pos1, Cell& pos2 );
    QStringList reverseList( const QStringList& ) const;
    QValueVector<QString> reverseVector( const QValueVector<QString>& ) const;
    void updateCellSize();
    QStringList vectorToList( const QValueVector<QString>& ) const;
    void updateIndexCache();

protected slots:
    void emitDateChange( int, int );
    void realDropEvent();
    void slotRepaint();

private:
    /**
     * The list of images shown
     */
    QValueVector<QString> _imageList;

    /**
     * A map mapping from filename to its index in _imageList.
     */
    QMap<QString,int> _fileNameMap;

    /**
     * When the user selects a date on the date bar the thumbnail view will
     * position itself accordingly. As a consequence, the thumbnail view
     * is telling the date bar which date it moved to. This is all fine
     * except for the fact that the date selected in the date bar, may be
     * for an image which is in the middle of a line, while the date
     * emitted from the thumbnail view is for the top most image in
     * the view (that is the first image on the line), which results in a
     * different cell being selected in the date bar, than what the user
     * selected.
     * Therefore we need this variable to disable the emission of the date
     * change while setting the date.
     */
    bool _isSettingDate;

    /*
     * This set contains the files currently selected.
     */
    Set<QString> _selectedFiles;

    /**
     * This is the item currently having keyboard focus
     *
     * We need to store the file name for the current item rather than its
     * coordinates, as coordinates changes when the grid is resized.
     */
    QString _currentItem;

    static ThumbnailWidget* _instance;

    ThumbnailToolTip* _toolTip;

    GridResizeInteraction _gridResizeInteraction;
    SelectionInteraction _selectionInteraction;
    MouseTrackingInteraction _mouseTrackingHandler;
    MouseInteraction* _mouseHandler;
    friend class GridResizeInteraction;
    friend class SelectionInteraction;
    friend class MouseTrackingInteraction;

    /**
     * file which should have drop indication point drawn on its left side
     */
    QString _leftDrop;

    /**
     * file which should have drop indication point drawn on its right side
     */
    QString _rightDrop;

    QTimer* _repaintTimer;

    Set<QString> _pendingRepaint;

    SortDirection _sortDirection;
};

}


#endif /* THUMBNAILVIEW_H */

