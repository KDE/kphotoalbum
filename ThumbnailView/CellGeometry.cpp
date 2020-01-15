/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#include "CellGeometry.h"

#include "ThumbnailModel.h"
#include "ThumbnailWidget.h"

#include <Settings/SettingsData.h>

#include <KLocalizedString>

using Utilities::StringSet;

ThumbnailView::CellGeometry::CellGeometry(ThumbnailFactory *factory)
    : ThumbnailComponent(factory)
    , m_cacheInitialized(false)
{
}

/**
 * Return desired size of the pixmap
 */
QSize ThumbnailView::CellGeometry::preferredIconSize()
{
    int width = Settings::SettingsData::instance()->actualThumbnailSize();
    int height = width * Settings::SettingsData::instance()->getThumbnailAspectRatio();
    return QSize(width, height);
}

/**
 * Return base size of the pixmap.
 * I.e. the unscaled thumbnail size, as it is set in the settings page.
 */
QSize ThumbnailView::CellGeometry::baseIconSize()
{
    int width = Settings::SettingsData::instance()->thumbnailSize();
    int height = width * Settings::SettingsData::instance()->getThumbnailAspectRatio();
    return QSize(width, height);
}

/**
 * Return the geometry for the icon in the cell. The coordinates are relative to the cell.
 */
QRect ThumbnailView::CellGeometry::iconGeometry(const QPixmap &pixmap) const
{
    const QSize cellSize = preferredIconSize();
    const int space = Settings::SettingsData::instance()->thumbnailSpace() + 5; /* 5 pixels for 3d effect */
    int width = cellSize.width() - space;

    int xoff = space / 2 + qMax(0, (width - pixmap.width()) / 2);
    int yoff = space / 2 + cellSize.height() - pixmap.height();
    return QRect(QPoint(xoff, yoff), pixmap.size());
}

/**
 * return the number of categories with values in for the given image.
 */
static int noOfCategoriesForImage(const DB::FileName &image)
{
    static const QString folder(i18n("Folder"));
    DB::ImageInfoPtr info = image.info();
    int grps = info->availableCategories().length();
    if (info->itemsOfCategory(folder).empty())
        return grps - 1;
    else
        return grps - 2; // Exclude folder and media type
}

/**
 * Return the height of the text under the thumbnails.
 */
int ThumbnailView::CellGeometry::textHeight() const
{
    if (!m_cacheInitialized)
        const_cast<CellGeometry *>(this)->flushCache();

    return m_textHeight;
}

QSize ThumbnailView::CellGeometry::cellSize() const
{
    if (!m_cacheInitialized)
        const_cast<CellGeometry *>(this)->flushCache();
    return m_cellSize;
}

QRect ThumbnailView::CellGeometry::cellTextGeometry() const
{
    if (!m_cacheInitialized)
        const_cast<CellGeometry *>(this)->flushCache();
    return m_cellTextGeometry;
}

void ThumbnailView::CellGeometry::flushCache()
{
    m_cacheInitialized = true;
    calculateTextHeight();
    calculateCellSize();
    calculateCellTextGeometry();
}

void ThumbnailView::CellGeometry::calculateTextHeight()
{
    m_textHeight = 0;

    const int charHeight = QFontMetrics(widget()->font()).height();
    if (Settings::SettingsData::instance()->displayLabels())
        m_textHeight += charHeight + 2;

    if (Settings::SettingsData::instance()->displayCategories()) {
        int maxCatsInText = 0;
        Q_FOREACH (const DB::FileName &fileName, model()->imageList(ViewOrder)) {
            maxCatsInText = qMax(noOfCategoriesForImage(fileName), maxCatsInText);
        }

        m_textHeight += charHeight * maxCatsInText + 5;
    }
}

void ThumbnailView::CellGeometry::calculateCellSize()
{
    const QSize iconSize = preferredIconSize();
    const int height = iconSize.height() + 2 + m_textHeight;
    const int space = Settings::SettingsData::instance()->thumbnailSpace() + 5; /* 5 pixels for 3d effect */
    m_cellSize = QSize(iconSize.width() + space, height + space);
}

void ThumbnailView::CellGeometry::calculateCellTextGeometry()
{
    if (!Settings::SettingsData::instance()->displayLabels() && !Settings::SettingsData::instance()->displayCategories())
        m_cellTextGeometry = QRect();
    else {
        const int h = m_textHeight;
        m_cellTextGeometry = QRect(1, m_cellSize.height() - h - 1, m_cellSize.width() - 2, h);
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
