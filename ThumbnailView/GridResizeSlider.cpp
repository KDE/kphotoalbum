/* SPDX-FileCopyrightText: 2015-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

// Qt includes
#include <QTimer>

// KDE includes
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

// Local includes
#include "CellGeometry.h"
#include "GridResizeSlider.h"
#include "Logging.h"
#include "ThumbnailModel.h"
#include "ThumbnailWidget.h"

#include <ImageManager/ThumbnailBuilder.h>
#include <MainWindow/Window.h>
#include <Settings/SettingsData.h>

ThumbnailView::GridResizeSlider::GridResizeSlider(ThumbnailFactory *factory)
    : QSlider(Qt::Horizontal)
    , ThumbnailComponent(factory)
{
    Settings::SettingsData *settings = Settings::SettingsData::instance();
    setMinimum(settings->minimumThumbnailSize());
    setMaximum(settings->thumbnailSize());
    setValue(settings->actualThumbnailSize());

    // timer for event-timeout:
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);

    // we have no definitive leave event when using the mousewheel -> use a timeout
    connect(m_timer, &QTimer::timeout, this, &GridResizeSlider::leaveGridResizingMode);

    connect(settings, &Settings::SettingsData::actualThumbnailSizeChanged, this, &GridResizeSlider::setValue);
    connect(settings, &Settings::SettingsData::thumbnailSizeChanged, this, &GridResizeSlider::setMaximum);

    connect(this, &GridResizeSlider::sliderPressed, this, &GridResizeSlider::enterGridResizingMode);
    connect(this, &GridResizeSlider::valueChanged, this, &GridResizeSlider::setCellSize);

    // disable drawing of thumbnails while resizing:
    connect(this, SIGNAL(isResizing(bool)), widget(), SLOT(setExternallyResizing(bool)));
}

ThumbnailView::GridResizeSlider::~GridResizeSlider()
{
    delete m_timer;
}

void ThumbnailView::GridResizeSlider::mousePressEvent(QMouseEvent *event)
{
    qCDebug(ThumbnailViewLog) << "Mouse pressed";
    enterGridResizingMode();
    QSlider::mousePressEvent(event);
}

void ThumbnailView::GridResizeSlider::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(ThumbnailViewLog) << "Mouse released";
    leaveGridResizingMode();
    QSlider::mouseReleaseEvent(event);
}

void ThumbnailView::GridResizeSlider::wheelEvent(QWheelEvent *event)
{
    // set (or reset) the timer to leave resizing mode:
    m_timer->start(200);
    qCDebug(ThumbnailViewLog) << "(Re)starting timer";
    if (!m_resizing) {
        enterGridResizingMode();
    }
    QSlider::wheelEvent(event);
}

void ThumbnailView::GridResizeSlider::enterGridResizingMode()
{
    if (m_resizing)
        return; //already resizing
    m_resizing = true;

    qCDebug(ThumbnailViewLog) << "Entering grid resizing mode";
    ImageManager::ThumbnailBuilder::instance()->cancelRequests();
    emit isResizing(true);
}

void ThumbnailView::GridResizeSlider::leaveGridResizingMode()
{
    if (!m_resizing)
        return; //not resizing
    m_resizing = false;
    qCDebug(ThumbnailViewLog) << "Leaving grid resizing mode";

    model()->beginResetModel();
    cellGeometryInfo()->flushCache();
    model()->endResetModel();
    model()->updateVisibleRowInfo();
    emit isResizing(false);
}

void ThumbnailView::GridResizeSlider::setCellSize(int size)
{
    blockSignals(true);
    Settings::SettingsData::instance()->setActualThumbnailSize(size);
    blockSignals(false);

    model()->beginResetModel();
    cellGeometryInfo()->calculateCellSize();
    model()->endResetModel();
}

void ThumbnailView::GridResizeSlider::setMaximum(int size)
{
    // QSlider::setMaximum() is not a slot, which is why we need this slot as workaround
    QSlider::setMaximum(size);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
