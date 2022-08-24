// SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "CellGeometry.h"

#include "ThumbnailModel.h"
#include "ThumbnailWidget.h"

#include <DB/ImageDB.h>
#include <kpabase/SettingsData.h>

#include <KLocalizedString>

using Utilities::StringSet;

ThumbnailView::CellGeometry::CellGeometry(ThumbnailFactory *factory)
    : ThumbnailComponent(factory)
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
    static const QString folder(i18n("Folder"));
    m_textHeight = 0;

    const int charHeight = QFontMetrics(widget()->font()).height();
    if (Settings::SettingsData::instance()->displayLabels())
        m_textHeight += charHeight + 2;

    if (Settings::SettingsData::instance()->displayCategories()) {
        int maxCatsInText = 0;
        const auto images = model()->imageList(ViewOrder);
        for (const DB::FileName &fileName : images) {
            const DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
            int grps = info->availableCategories().length();
            if (grps > maxCatsInText - 2) {
                grps -= info->itemsOfCategory(folder).empty() ? 1 : 2;
                if (grps > maxCatsInText)
                    maxCatsInText = grps;
            }
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
