// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021-2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef THUMBNAILVIEW_THUMBNAILWIDGET_H
#define THUMBNAILVIEW_THUMBNAILWIDGET_H

#include "GridResizeInteraction.h"
#include "MouseTrackingInteraction.h"
#include "SelectionInteraction.h"
#include "ThumbnailComponent.h"
#include "VideoThumbnailCycler.h"
#include "enums.h"

#include <QListView>
#include <QScopedPointer>

class QTimer;
namespace Utilities
{
class FastDateTime;
}

namespace DB
{
class ImageDate;
class Id;
class FileNameList;
}

namespace ThumbnailView
{
class ThumbnailPainter;
class CellGeometry;
class ThumbnailModel;
class ThumbnailFactory;
class KeyboardEventHandler;
class ThumbnailDND;

class ThumbnailWidget : public QListView, private ThumbnailComponent
{
    Q_OBJECT

public:
    explicit ThumbnailWidget(ThumbnailFactory *factory);

    void reload(SelectionUpdateMethod method);
    DB::FileName mediaIdUnderCursor() const;
    QModelIndex indexUnderCursor() const;

    bool isMouseOverStackIndicator(const QPoint &point);
    bool isGridResizing() const;
    void setCurrentItem(const DB::FileName &fileName);
    DB::FileName currentItem() const;
    void changeSingleSelection(const DB::FileName &fileName);

    // Misc
    int cellWidth() const;
    void showEvent(QShowEvent *) override;
    DB::FileNameList selection(ThumbnailView::SelectionMode mode) const;
    bool isSelected(const DB::FileName &id) const;
    void select(const DB::FileNameList &);
    bool isItemUnderCursorSelected() const;

public Q_SLOTS:
    void gotoDate(const DB::ImageDate &date, bool includeRanges);
    /**
     * @brief setExternallyResizing
     * Used by the GridResizeSlider to indicate that the grid is being resized.
     * @param state true, if the grid is being resized by an external widget, false if not
     */
    void setExternallyResizing(bool state);

Q_SIGNALS:
    void showImage(const DB::FileName &id);
    void showSelection();
    void showSearch();
    void fileIdUnderCursorChanged(const DB::FileName &id);
    void currentDateChanged(const Utilities::FastDateTime &);
    void selectionCountChanged(int numberOfItemsSelected);

protected:
    // event handlers
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;

    // Drag and drop
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *) override;
    void dragLeaveEvent(QDragLeaveEvent *) override;
    void dropEvent(QDropEvent *) override;

private Q_SLOTS:
    void emitDateChange();
    void scheduleDateChangeSignal();
    void emitSelectionChangedSignal();

private:
    friend class GridResizeInteraction;
    inline ThumbnailModel *model() { return ThumbnailComponent::model(); }
    inline const ThumbnailModel *model() const { return ThumbnailComponent::model(); }
    void updatePalette();
    void setupDateChangeTimer();

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
    bool m_isSettingDate;

    GridResizeInteraction m_gridResizeInteraction;
    bool m_wheelResizing;
    bool m_externallyResizing;
    SelectionInteraction m_selectionInteraction;
    MouseTrackingInteraction m_mouseTrackingHandler;
    MouseInteraction *m_mouseHandler;
    ThumbnailDND *m_dndHandler;
    bool m_pressOnStackIndicator;

    QTimer *m_dateChangedTimer;

    friend class SelectionInteraction;
    friend class KeyboardEventHandler;
    friend class ThumbnailDND;
    friend class ThumbnailModel;
    KeyboardEventHandler *m_keyboardHandler;
    QScopedPointer<VideoThumbnailCycler> m_videoThumbnailCycler;
};
}

#endif /* THUMBNAILVIEW_THUMBNAILWIDGET_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
