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

#include "ThumbnailWidget.h"

#include "CellGeometry.h"
#include "Delegate.h"
#include "KeyboardEventHandler.h"
#include "SelectionMaintainer.h"
#include "ThumbnailDND.h"
#include "ThumbnailFactory.h"
#include "ThumbnailModel.h"

#include <Browser/BrowserWidget.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfoPtr.h>
#include <Settings/SettingsData.h>

#include <KColorScheme>
#include <KLocalizedString>
#include <QCursor>
#include <QDebug>
#include <QFontMetrics>
#include <QItemSelection>
#include <QItemSelectionRange>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <math.h>

/**
 * \class ThumbnailView::ThumbnailWidget
 * This is the widget which shows the thumbnails.
 *
 * In previous versions this was implemented using a QIconView, but there
 * simply was too many problems, so after years of tears and pains I
 * rewrote it.
 */

ThumbnailView::ThumbnailWidget::ThumbnailWidget(ThumbnailFactory *factory)
    : QListView()
    , ThumbnailComponent(factory)
    , m_isSettingDate(false)
    , m_gridResizeInteraction(factory)
    , m_wheelResizing(false)
    , m_externallyResizing(false)
    , m_selectionInteraction(factory)
    , m_mouseTrackingHandler(factory)
    , m_mouseHandler(&m_mouseTrackingHandler)
    , m_dndHandler(new ThumbnailDND(factory))
    , m_pressOnStackIndicator(false)
    , m_keyboardHandler(new KeyboardEventHandler(factory))
    , m_videoThumbnailCycler(new VideoThumbnailCycler(model()))
{
    setModel(ThumbnailComponent::model());
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setUniformItemSizes(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    // It beats me why I need to set mouse tracking on both, but without it doesn't work.
    viewport()->setMouseTracking(true);
    setMouseTracking(true);

    connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &ThumbnailWidget::scheduleDateChangeSignal);
    viewport()->setAcceptDrops(true);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(&m_mouseTrackingHandler, &MouseTrackingInteraction::fileIdUnderCursorChanged, this, &ThumbnailWidget::fileIdUnderCursorChanged);
    connect(m_keyboardHandler, &KeyboardEventHandler::showSelection, this, &ThumbnailWidget::showSelection);

    updatePalette();
    connect(Settings::SettingsData::instance(), &Settings::SettingsData::colorSchemeChanged, this, &ThumbnailWidget::updatePalette);
    setItemDelegate(new Delegate(factory, this));

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &ThumbnailWidget::emitSelectionChangedSignal);

    setDragEnabled(false); // We run our own dragging, so disable QListView's version.

    connect(verticalScrollBar(), &QScrollBar::valueChanged, model(), &ThumbnailModel::updateVisibleRowInfo);
    setupDateChangeTimer();
}

bool ThumbnailView::ThumbnailWidget::isGridResizing() const
{
    return m_mouseHandler->isResizingGrid() || m_wheelResizing || m_externallyResizing;
}

void ThumbnailView::ThumbnailWidget::keyPressEvent(QKeyEvent *event)
{
    if (!m_keyboardHandler->keyPressEvent(event))
        QListView::keyPressEvent(event);
}

void ThumbnailView::ThumbnailWidget::keyReleaseEvent(QKeyEvent *event)
{
    const bool propagate = m_keyboardHandler->keyReleaseEvent(event);
    if (propagate)
        QListView::keyReleaseEvent(event);
}

bool ThumbnailView::ThumbnailWidget::isMouseOverStackIndicator(const QPoint &point)
{
    // first check if image is stack, if not return.
    DB::ImageInfoPtr imageInfo = mediaIdUnderCursor().info();
    if (!imageInfo)
        return false;
    if (!imageInfo->isStacked())
        return false;

    const QModelIndex index = indexUnderCursor();
    const QRect itemRect = visualRect(index);
    const QPixmap pixmap = index.data(Qt::DecorationRole).value<QPixmap>();
    if (pixmap.isNull())
        return false;

    const QRect pixmapRect = cellGeometryInfo()->iconGeometry(pixmap).translated(itemRect.topLeft());
    const QRect blackOutRect = pixmapRect.adjusted(0, 0, -10, -10);
    return pixmapRect.contains(point) && !blackOutRect.contains(point);
}

static bool isMouseResizeGesture(QMouseEvent *event)
{
    return (event->button() & Qt::MidButton) || ((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::AltModifier));
}

void ThumbnailView::ThumbnailWidget::mousePressEvent(QMouseEvent *event)
{
    if ((!(event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) && isMouseOverStackIndicator(event->pos())) {
        model()->toggleStackExpansion(mediaIdUnderCursor());
        m_pressOnStackIndicator = true;
        return;
    }

    if (isMouseResizeGesture(event))
        m_mouseHandler = &m_gridResizeInteraction;
    else
        m_mouseHandler = &m_selectionInteraction;

    if (!m_mouseHandler->mousePressEvent(event))
        QListView::mousePressEvent(event);

    if (event->button() & Qt::RightButton) //get out of selection mode if this is a right click
        m_mouseHandler = &m_mouseTrackingHandler;
}

void ThumbnailView::ThumbnailWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressOnStackIndicator)
        return;

    if (!m_mouseHandler->mouseMoveEvent(event))
        QListView::mouseMoveEvent(event);
}

void ThumbnailView::ThumbnailWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_pressOnStackIndicator) {
        m_pressOnStackIndicator = false;
        return;
    }

    if (!m_mouseHandler->mouseReleaseEvent(event))
        QListView::mouseReleaseEvent(event);

    m_mouseHandler = &m_mouseTrackingHandler;
}

void ThumbnailView::ThumbnailWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (isMouseOverStackIndicator(event->pos())) {
        model()->toggleStackExpansion(mediaIdUnderCursor());
        m_pressOnStackIndicator = true;
    } else if (!(event->modifiers() & Qt::ControlModifier)) {
        DB::FileName id = mediaIdUnderCursor();
        if (!id.isNull())
            emit showImage(id);
    }
}

void ThumbnailView::ThumbnailWidget::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        event->setAccepted(true);
        if (!m_wheelResizing)
            m_gridResizeInteraction.enterGridResizingMode();

        m_wheelResizing = true;

        model()->beginResetModel();
        const int delta = -event->delta() / 20;
        static int _minimum_ = Settings::SettingsData::instance()->minimumThumbnailSize();
        Settings::SettingsData::instance()->setActualThumbnailSize(qMax(_minimum_, Settings::SettingsData::instance()->actualThumbnailSize() + delta));
        cellGeometryInfo()->calculateCellSize();
        model()->endResetModel();
    } else {
        int delta = event->delta() / 5;
        QWheelEvent newevent = QWheelEvent(event->pos(), delta, event->buttons(), nullptr);

        QListView::wheelEvent(&newevent);
    }
}

void ThumbnailView::ThumbnailWidget::emitDateChange()
{
    if (m_isSettingDate)
        return;

    int row = currentIndex().row();
    if (row == -1)
        return;

    DB::FileName fileName = model()->imageAt(row);
    if (fileName.isNull())
        return;

    static QDateTime lastDate;
    QDateTime date = fileName.info()->date().start();
    if (date != lastDate) {
        lastDate = date;
        if (date.date().year() != 1900)
            emit currentDateChanged(date);
    }
}

/**
 * scroll to the date specified with the parameter date.
 * The boolean includeRanges tells whether we accept range matches or not.
 */
void ThumbnailView::ThumbnailWidget::gotoDate(const DB::ImageDate &date, bool includeRanges)
{
    m_isSettingDate = true;
    DB::FileName candidate = DB::ImageDB::instance()
                                 ->findFirstItemInRange(model()->imageList(ViewOrder), date, includeRanges);
    if (!candidate.isNull())
        setCurrentItem(candidate);

    m_isSettingDate = false;
}

void ThumbnailView::ThumbnailWidget::setExternallyResizing(bool state)
{
    m_externallyResizing = state;
}

void ThumbnailView::ThumbnailWidget::reload(SelectionUpdateMethod method)
{
    SelectionMaintainer maintainer(this, model());
    ThumbnailComponent::model()->beginResetModel();
    cellGeometryInfo()->flushCache();
    updatePalette();
    ThumbnailComponent::model()->endResetModel();

    if (method == ClearSelection)
        maintainer.disable();
}

DB::FileName ThumbnailView::ThumbnailWidget::mediaIdUnderCursor() const
{
    const QModelIndex index = indexUnderCursor();
    if (index.isValid())
        return model()->imageAt(index.row());
    else
        return DB::FileName();
}

QModelIndex ThumbnailView::ThumbnailWidget::indexUnderCursor() const
{
    return indexAt(mapFromGlobal(QCursor::pos()));
}

void ThumbnailView::ThumbnailWidget::dragMoveEvent(QDragMoveEvent *event)
{
    m_dndHandler->contentsDragMoveEvent(event);
}

void ThumbnailView::ThumbnailWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_dndHandler->contentsDragLeaveEvent(event);
}

void ThumbnailView::ThumbnailWidget::dropEvent(QDropEvent *event)
{
    m_dndHandler->contentsDropEvent(event);
}

void ThumbnailView::ThumbnailWidget::dragEnterEvent(QDragEnterEvent *event)
{
    m_dndHandler->contentsDragEnterEvent(event);
}

void ThumbnailView::ThumbnailWidget::setCurrentItem(const DB::FileName &fileName)
{
    if (fileName.isNull())
        return;

    const int row = model()->indexOf(fileName);
    setCurrentIndex(QListView::model()->index(row, 0));
}

DB::FileName ThumbnailView::ThumbnailWidget::currentItem() const
{
    if (!currentIndex().isValid())
        return DB::FileName();

    return model()->imageAt(currentIndex().row());
}

void ThumbnailView::ThumbnailWidget::updatePalette()
{
    QPalette pal = palette();
    // if the scheme was set at startup from the scheme path (and not afterwards through KColorSchemeManager),
    // then KColorScheme would use the standard system scheme if we don't explicitly give a config:
    const auto schemeCfg = KSharedConfig::openConfig(Settings::SettingsData::instance()->colorScheme());
    KColorScheme::adjustBackground(pal, KColorScheme::NormalBackground, QPalette::Base, KColorScheme::Complementary, schemeCfg);
    KColorScheme::adjustForeground(pal, KColorScheme::NormalText, QPalette::Text, KColorScheme::Complementary, schemeCfg);
    setPalette(pal);
}

int ThumbnailView::ThumbnailWidget::cellWidth() const
{
    return visualRect(QListView::model()->index(0, 0)).size().width();
}

void ThumbnailView::ThumbnailWidget::emitSelectionChangedSignal()
{
    emit selectionCountChanged(selection(ExpandCollapsedStacks).size());
}

void ThumbnailView::ThumbnailWidget::scheduleDateChangeSignal()
{
    m_dateChangedTimer->start(200);
}

/**
 * During profiling, I found that emitting the dateChanged signal was
 * rather expensive, so now I delay that signal, so it is only emitted 200
 * msec after the scroll, which means it will not be emitted when the user
 * holds down, say the page down key for scrolling.
 */
void ThumbnailView::ThumbnailWidget::setupDateChangeTimer()
{
    m_dateChangedTimer = new QTimer(this);
    m_dateChangedTimer->setSingleShot(true);
    connect(m_dateChangedTimer, &QTimer::timeout, this, &ThumbnailWidget::emitDateChange);
}

void ThumbnailView::ThumbnailWidget::showEvent(QShowEvent *event)
{
    model()->updateVisibleRowInfo();
    QListView::showEvent(event);
}

DB::FileNameList ThumbnailView::ThumbnailWidget::selection(ThumbnailView::SelectionMode mode) const
{
    DB::FileNameList res;
    const auto indexSelection = selectedIndexes();
    for (const QModelIndex &index : indexSelection) {
        const DB::FileName currFileName = model()->imageAt(index.row());
        bool includeAllStacks = false;
        switch (mode) {
        case IncludeAllStacks:
            includeAllStacks = true;
            /* FALLTHROUGH */
        case ExpandCollapsedStacks: {
            // if the selected image belongs to a collapsed thread,
            // imply that all images in the stack are selected:
            DB::ImageInfoPtr imageInfo = currFileName.info();
            if (imageInfo && imageInfo->isStacked()
                && (includeAllStacks || !model()->isItemInExpandedStack(imageInfo->stackId()))) {
                // add all images in the same stack
                res.append(DB::ImageDB::instance()->getStackFor(currFileName));
            } else
                res.append(currFileName);
        } break;
        case NoExpandCollapsedStacks:
            res.append(currFileName);
            break;
        }
    }
    return res;
}

bool ThumbnailView::ThumbnailWidget::isSelected(const DB::FileName &fileName) const
{
    return selection(NoExpandCollapsedStacks).indexOf(fileName) != -1;
}

/**
   This very specific method will make the item specified by id selected,
   if there only are one item selected. This is used from the Viewer when
   you start it without a selection, and are going forward or backward.
*/
void ThumbnailView::ThumbnailWidget::changeSingleSelection(const DB::FileName &fileName)
{
    if (selection(NoExpandCollapsedStacks).size() == 1) {
        QItemSelectionModel *selection = selectionModel();
        selection->select(model()->fileNameToIndex(fileName), QItemSelectionModel::ClearAndSelect);
        setCurrentItem(fileName);
    }
}

void ThumbnailView::ThumbnailWidget::select(const DB::FileNameList &items)
{
    QItemSelection selection;
    QModelIndex start;
    QModelIndex end;
    int count = 0;
    for (const DB::FileName &fileName : items) {
        QModelIndex index = model()->fileNameToIndex(fileName);
        if (count == 0) {
            start = index;
            end = index;
        } else if (index.row() == end.row() + 1) {
            end = index;
        } else {
            selection.merge(QItemSelection(start, end), QItemSelectionModel::Select);
            start = index;
            end = index;
        }
        count++;
    }
    if (count > 0) {
        selection.merge(QItemSelection(start, end), QItemSelectionModel::Select);
    }
    selectionModel()->select(selection, QItemSelectionModel::Select);
}

bool ThumbnailView::ThumbnailWidget::isItemUnderCursorSelected() const
{
    return widget()->selection(ExpandCollapsedStacks).contains(mediaIdUnderCursor());
}

// vi:expandtab:tabstop=4 shiftwidth=4:
