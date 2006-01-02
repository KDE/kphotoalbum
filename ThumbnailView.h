#ifndef THUMBNAILVIEW_H
#define THUMBNAILVIEW_H

#include <qgridview.h>
#include <qvaluelist.h>
#include "imageclient.h"
#include "set.h"
#include "ThumbnailToolTip.h"
class ImageDateRange;
class QDateTime;
class QPixmapCache;

class ThumbnailView : public QGridView, public ImageClient {
    Q_OBJECT

public:
    ThumbnailView( QWidget* parent, const char* name = 0 );
    void setImageList( const QStringList& list );

    virtual void paintCell ( QPainter * p, int row, int col );
    virtual void pixmapLoaded( const QString&, const QSize& size, const QSize& fullSize, int, const QImage&, bool loadedOK );
    bool thumbnailStillNeeded( const QString& fileName ) const;
    QStringList selection() const;
    QStringList imageList() const;
    void reload();
    QString fileNameUnderCursor() const;
    QString currentItem() const;
    static ThumbnailView* theThumbnailView();
    void makeCurrent( const QString& fileName );

public slots:
    void gotoDate( const ImageDateRange& date, bool includeRanges );
    void selectAll();
    void showToolTipsOnImages( bool b );

signals:
    void showImage( const QString& fileName );
    void fileNameUnderCursorChanged( const QString& fileName );
    void currentDateChanged( const QDateTime& );
    void selectionChanged();

protected:
    // Painting
    void repaintCell( const QString& fileName );
    void repaintCell( int row, int col );
    void paintCellBackground( QPainter* p, int row, int col );
    void repaintScreen();

    // Cell handling methods.
    QString fileNameInCell( int row, int col ) const;
    QString fileNameInCell( const QPoint& cell ) const;
    QString fileNameAtViewportPos( const QPoint& pos ) const;
    QPoint positionForFileName( const QString& fileName ) const;
    QPoint cellAtViewportPos( const QPoint& pos ) const;

    enum VisibleState { FullyVisible, PartlyVisible };
    int firstVisibleRow( VisibleState ) const;
    int lastVisibleRow( VisibleState ) const;
    int thumbnailsPerRow() const;
    int numRowsPerPage() const;
    QRect iconGeometry( int row, int col ) const;
    bool isFocusAtFirstCell() const;
    bool isFocusAtLastCell() const;
    QPoint lastCell() const;

    // event handlers
    virtual void keyPressEvent( QKeyEvent* );
    virtual void showEvent( QShowEvent* );
    virtual void mousePressEvent( QMouseEvent* );
    virtual void mouseMoveEvent( QMouseEvent* );
    virtual void mouseReleaseEvent( QMouseEvent* );
    virtual void mouseDoubleClickEvent ( QMouseEvent* );
    virtual void resizeEvent( QResizeEvent* );
    void keyboardMoveEvent( QKeyEvent* );

    // Selection
    void selectAllCellsBetween( QPoint pos1, QPoint pos2 );
    void selectCell( int row, int col );
    void selectCell( const QPoint& );
    void handleDragSelection();
    void clearSelection();
    void toggleSelection( const QString& fileName );
    void possibleEmitSelectionChanged();

    // Misc
    QPixmapCache& pixmapCache();
    void updateGridSize();
    bool isMovementKey( int key );

protected slots:
    void emitDateChange( int, int );

private:
    QValueList<QString> _imageList;

    /**
     * This variable contains the position the mouse was pressed down. This
     * is used for selection.
     *
     * The point is in contents coordinates.
     *
     * Auto scroll prevents us from using _mousePressPosViewport for this.
     */
    QPoint _mousePressPosContents;

    /**
     * This variable contains the position the mouse was pressed down. This
     * is used to calculate the grid size when resizing grid.
     *
     * The point is in viewport coordinates.
     * It is not possible to used the _mousePressPosContents instance
     * variable for this, since the view is scroll at soon as a resize is started.
     */
    QPoint _mousePressPosViewport;

    /**
     * This variable contains the size of a cell prior to the beginning of
     * resizing the grid.
     */
    int _origSize;

    /**
     * During resizing the size of the thumbnails, thumbnails should not be
     * displayed. This variable indicates whether we are resizing the view
     * or not.
     */
    bool _isResizing;

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

    Set<QString> _originalSelectionBeforeDragStart ;

    static ThumbnailView* _instance;

    ThumbnailToolTip* _toolTip;
};



#endif /* THUMBNAILVIEW_H */

