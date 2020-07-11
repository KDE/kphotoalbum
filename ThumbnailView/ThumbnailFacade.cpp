/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

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
#include "ThumbnailFacade.h"

#include "CellGeometry.h"
#include "FilterWidget.h"
#include "GridResizeSlider.h"
#include "ThumbnailModel.h"
#include "ThumbnailToolTip.h"
#include "ThumbnailWidget.h"

#include <BackgroundJobs/HandleVideoThumbnailRequestJob.h>
#include <ImageManager/ThumbnailCache.h>
#include <Settings/SettingsData.h>

ThumbnailView::ThumbnailFacade *ThumbnailView::ThumbnailFacade::s_instance = nullptr;
ThumbnailView::ThumbnailFacade::ThumbnailFacade(ImageManager::ThumbnailCache *thumbnailCache)
    : m_cellGeometry(nullptr)
    , m_model(nullptr)
    , m_widget(nullptr)
    , m_toolTip(nullptr)
    , m_thumbnailCache(thumbnailCache)
{
    // To avoid one of the components references one of the other before it has been initialized, we first construct them all with null.
    m_cellGeometry = new CellGeometry(this);
    m_model = new ThumbnailModel(this, m_thumbnailCache);
    m_widget = new ThumbnailWidget(this);
    // I don't want to introduce a strong interdependency between ThumbnailWidget and ThumbnailToolTip,
    // hence I don't want to push the tooltip construction into the thumbnail widget.
    // Unfortunately, this causes a warning in lgtm:
    m_toolTip = new ThumbnailToolTip(m_widget); // lgtm [cpp/resource-not-released-in-destructor]

    connect(m_widget, &ThumbnailWidget::showImage, this, &ThumbnailFacade::showImage);
    connect(m_widget, &ThumbnailWidget::showSelection, this, &ThumbnailFacade::showSelection);
    connect(m_widget, &ThumbnailWidget::fileIdUnderCursorChanged, this, &ThumbnailFacade::fileIdUnderCursorChanged);
    connect(m_widget, &ThumbnailWidget::currentDateChanged, this, &ThumbnailFacade::currentDateChanged);
    connect(m_widget, &ThumbnailWidget::selectionCountChanged, this, &ThumbnailFacade::selectionChanged);
    connect(m_model, &ThumbnailModel::collapseAllStacksEnabled, this, &ThumbnailFacade::collapseAllStacksEnabled);
    connect(m_model, &ThumbnailModel::expandAllStacksEnabled, this, &ThumbnailFacade::expandAllStacksEnabled);

    s_instance = this;
}

QWidget *ThumbnailView::ThumbnailFacade::gui()
{
    return m_widget;
}

void ThumbnailView::ThumbnailFacade::gotoDate(const DB::ImageDate &date, bool b)
{
    m_widget->gotoDate(date, b);
}

void ThumbnailView::ThumbnailFacade::setCurrentItem(const DB::FileName &fileName)
{
    widget()->setCurrentItem(fileName);
}

void ThumbnailView::ThumbnailFacade::reload(SelectionUpdateMethod method)
{
    m_widget->reload(method);
}

DB::FileNameList ThumbnailView::ThumbnailFacade::selection(ThumbnailView::SelectionMode mode) const
{
    return m_widget->selection(mode);
}

DB::FileNameList ThumbnailView::ThumbnailFacade::imageList(Order order) const
{
    return m_model->imageList(order);
}

DB::FileName ThumbnailView::ThumbnailFacade::mediaIdUnderCursor() const
{
    return m_widget->mediaIdUnderCursor();
}

DB::FileName ThumbnailView::ThumbnailFacade::currentItem() const
{
    return m_model->imageAt(m_widget->currentIndex().row());
}

void ThumbnailView::ThumbnailFacade::setImageList(const DB::FileNameList &list)
{
    m_model->setImageList(list);
}

void ThumbnailView::ThumbnailFacade::setSortDirection(SortDirection direction)
{
    m_model->setSortDirection(direction);
}

QSlider *ThumbnailView::ThumbnailFacade::createResizeSlider()
{
    return new GridResizeSlider(this);
}

ThumbnailView::FilterWidget *ThumbnailView::ThumbnailFacade::createFilterWidget(QWidget *parent)
{
    return model()->createFilterWidget(parent);
}

void ThumbnailView::ThumbnailFacade::selectAll()
{
    m_widget->selectAll();
}

void ThumbnailView::ThumbnailFacade::clearSelection()
{
    m_widget->clearSelection();
}

void ThumbnailView::ThumbnailFacade::showToolTipsOnImages(bool on)
{
    m_toolTip->setActive(on);
}

void ThumbnailView::ThumbnailFacade::toggleStackExpansion(const DB::FileName &fileName)
{
    m_model->toggleStackExpansion(fileName);
}

void ThumbnailView::ThumbnailFacade::collapseAllStacks()
{
    m_model->collapseAllStacks();
}

void ThumbnailView::ThumbnailFacade::expandAllStacks()
{
    m_model->expandAllStacks();
}

void ThumbnailView::ThumbnailFacade::updateDisplayModel()
{
    m_model->updateDisplayModel();
}

void ThumbnailView::ThumbnailFacade::changeSingleSelection(const DB::FileName &fileName)
{
    m_widget->changeSingleSelection(fileName);
}

ThumbnailView::ThumbnailModel *ThumbnailView::ThumbnailFacade::model()
{
    Q_ASSERT(m_model);
    return m_model;
}

ThumbnailView::CellGeometry *ThumbnailView::ThumbnailFacade::cellGeometry()
{
    Q_ASSERT(m_cellGeometry);
    return m_cellGeometry;
}

ThumbnailView::ThumbnailWidget *ThumbnailView::ThumbnailFacade::widget()
{
    Q_ASSERT(m_widget);
    return m_widget;
}

ThumbnailView::ThumbnailFacade *ThumbnailView::ThumbnailFacade::instance()
{
    Q_ASSERT(s_instance);
    return s_instance;
}

void ThumbnailView::ThumbnailFacade::slotRecreateThumbnail()
{
    const auto selection = widget()->selection(NoExpandCollapsedStacks);
    for (const DB::FileName &fileName : selection) {
        m_thumbnailCache->removeThumbnail(fileName);
        BackgroundJobs::HandleVideoThumbnailRequestJob::removeFullScaleFrame(fileName);
        m_model->updateCell(fileName);
    }
}

void ThumbnailView::ThumbnailFacade::clearFilter()
{
    Q_ASSERT(m_model);
    m_model->clearFilter();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
