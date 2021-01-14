/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef THUMBNAILFACADE_H
#define THUMBNAILFACADE_H
#include "ThumbnailFactory.h"
#include "ThumbnailWidget.h"

#include <kpabase/FileNameList.h>

class KActionCollection;
class QSlider;

namespace ImageManager
{
class ThumbnailCache;
}

namespace ThumbnailView
{
class ThumbnailModel;
class CellGeometry;
class FilterWidget;
class ThumbnailPainter;
class ThumbnailToolTip;

class ThumbnailFacade : public QObject, public ThumbnailFactory
{
    Q_OBJECT
public:
    static ThumbnailFacade *instance();
    ThumbnailFacade(ImageManager::ThumbnailCache *thumbnailCache);
    QWidget *gui();
    void setCurrentItem(const DB::FileName &fileName);
    void reload(SelectionUpdateMethod method);
    DB::FileNameList selection(ThumbnailView::SelectionMode mode = ExpandCollapsedStacks) const;
    DB::FileNameList imageList(Order) const;
    DB::FileName mediaIdUnderCursor() const;
    DB::FileName currentItem() const;
    void setImageList(const DB::FileNameList &list);
    void setSortDirection(SortDirection);
    /**
     * @brief createResizeSlider returns a QSlider that can be used to resize the thumbnail grid.
     * @return a (horizontal) QSlider
     */
    QSlider *createResizeSlider();

    /**
     * @brief filterWidget provides a FilterWidget that is connected to the ThumbnailModel.
     * The widget will reflect changes in the filter and can be used to set the filter.
     * @return a FilterWidget
     */
    FilterWidget *createFilterWidget(QWidget *parent);

public slots:
    void gotoDate(const DB::ImageDate &date, bool includeRanges);
    void selectAll();
    void clearSelection();
    void showToolTipsOnImages(bool b);
    void toggleStackExpansion(const DB::FileName &id);
    void collapseAllStacks();
    void expandAllStacks();
    void updateDisplayModel();
    void changeSingleSelection(const DB::FileName &fileName);
    void slotRecreateThumbnail();

    void clearFilter();

signals:
    void showImage(const DB::FileName &id);
    void showSelection();
    void fileIdUnderCursorChanged(const DB::FileName &id);
    void currentDateChanged(const Utilities::FastDateTime &);
    void selectionChanged(int numberOfItemsSelected);
    void collapseAllStacksEnabled(bool enabled);
    void expandAllStacksEnabled(bool enabled);

private:
    ThumbnailModel *model() override;
    CellGeometry *cellGeometry() override;
    ThumbnailWidget *widget() override;

private:
    static ThumbnailFacade *s_instance;
    CellGeometry *m_cellGeometry;
    ThumbnailModel *m_model;
    ThumbnailWidget *m_widget;
    ThumbnailPainter *m_painter;
    ThumbnailToolTip *m_toolTip;
    ImageManager::ThumbnailCache *m_thumbnailCache;
};
}

#endif /* THUMBNAILFACADE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
