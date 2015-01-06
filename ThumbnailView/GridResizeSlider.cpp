/* Copyright (C) 2015 Johannes Zarl <johannes@zarl.at>

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
#include "GridResizeSlider.h"

#include <QScrollBar>
#include <KSharedConfig>

#include "CellGeometry.h"
#include "ThumbnailModel.h"
#include "ThumbnailWidget.h"
#include "Settings/SettingsData.h"
#include "ImageManager/ThumbnailBuilder.h"

ThumbnailView::GridResizeSlider::GridResizeSlider( ThumbnailFactory* factory )
    : QSlider( Qt::Horizontal ), ThumbnailComponent( factory )
{
    Settings::SettingsData *settings = Settings::SettingsData::instance();
    setMinimum( settings->minimumThumbnailSize() );
    setMaximum( settings->thumbnailSize() );
    setValue( settings->actualThumbnailSize() );

    connect( settings, SIGNAL(actualThumbnailSizeChanged(int)), this , SLOT(setValue(int)) );
    connect( settings, SIGNAL(thumbnailSizeChanged(int)), this, SLOT(setMaximum(int)) );

    connect( this, SIGNAL(sliderPressed()), this, SLOT(enterGridResizingMode()) );
    connect( this, SIGNAL(valueChanged(int)), this, SLOT(setCellSize(int)) );
}

void ThumbnailView::GridResizeSlider::enterGridResizingMode()
{
    ImageManager::ThumbnailBuilder::instance()->cancelRequests();
}

void ThumbnailView::GridResizeSlider::setCellSize(int size)
{
    blockSignals(true);
    Settings::SettingsData::instance()->setActualThumbnailSize( size );
    blockSignals(false);

    model()->reset();
    cellGeometryInfo()->flushCache();
    model()->updateVisibleRowInfo();
}

void ThumbnailView::GridResizeSlider::setMaximum(int size)
{
    // QSlider::setMaximum() is not a slot, which is why we need this slot as workaround
    QSlider::setMaximum(size);
}

#include "GridResizeSlider.moc"

// vi:expandtab:tabstop=4 shiftwidth=4:
