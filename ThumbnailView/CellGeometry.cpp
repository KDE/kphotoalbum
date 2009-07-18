/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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
#include "ThumbnailCache.h"
#include "ThumbnailModel.h"
#include "DB/ResultId.h"
#include "Settings/SettingsData.h"

using Utilities::StringSet;

ThumbnailView::CellGeometry::CellGeometry( ThumbnailFactory* factory )
    :ThumbnailComponent(factory)
{
}

/**
 * Return desired size of the whole cell
 */
QSize ThumbnailView::CellGeometry::cellSize() const
{
    int width = Settings::SettingsData::instance()->thumbSize();
    int height = width;

    switch (Settings::SettingsData::instance()->thumbnailAspectRatio()) {
        case Settings::Aspect_16_9:
	    height = (int) (height * 9.0 / 16);
	    break;
        case Settings::Aspect_4_3:
	    height = (int) (height * 3.0 / 4);
	    break;
        case Settings::Aspect_3_2:
	    height = (int) (height * 2.0 / 3);
	    break;
        case Settings::Aspect_9_16:
	    width = (int) (width * 9.0 / 16);
	    break;
        case Settings::Aspect_3_4:
	    width = (int) (width * 3.0 / 4);
	    break;
        case Settings::Aspect_2_3:
	    width = (int) (width * 2.0 / 3);
	    break;
	case Settings::Aspect_1_1:
	    // nothing
	    ;
    }
    return QSize( width, height);
}


/**
 * Return the geometry for the icon in the cell (row,col). The returned coordinates are local to the cell.
 */
QRect ThumbnailView::CellGeometry::iconGeometry( int row, int col ) const
{
    DB::ResultId mediaId = model()->imageAt( row, col );
    if ( mediaId.isNull() ) // empty cell
        return QRect();

    const QSize cellSize = this->cellSize();
    const int space = Settings::SettingsData::instance()->thumbnailSpace();
    int width = cellSize.width() - 2 * space;
    int height = cellSize.height() - 2 * space;

    QPixmap pixmap;
    if (!cache()->find(mediaId, &pixmap)
        || (pixmap.width() == 0 && pixmap.height() == 0)) {
        return QRect( space, space, width, height );
    }

    int xoff = space + (width - pixmap.width()) / 2;
    int yoff = space + (height - pixmap.height()) / 2;

    return QRect( xoff, yoff, pixmap.width(), pixmap.height() );
}

/**
 * return the number of categories with valies in for the given image.
 */
static int noOfCategoriesForImage(const DB::ResultId& image )
{
    int catsInText = 0;
    QStringList grps = image.fetchInfo()->availableCategories();
    for( QStringList::const_iterator it = grps.constBegin(); it != grps.constEnd(); ++it ) {
        QString category = *it;
        if ( category != QString::fromLatin1( "Folder" ) && category != QString::fromLatin1( "Media Type" ) ) {
            StringSet items = image.fetchInfo()->itemsOfCategory( category );
            if (!items.empty()) {
                catsInText++;
            }
        }
    }
    return catsInText;
}


/**
 * Return the height of the text under the thumbnails.
 */
int ThumbnailView::CellGeometry::textHeight( int charHeight, bool reCalc ) const
{
    int h = 0;
    static int maxCatsInText = 0;

    if ( Settings::SettingsData::instance()->displayLabels() )
        h += charHeight +2;
    if ( Settings::SettingsData::instance()->displayCategories()) {
        if ( reCalc ) {
            maxCatsInText = 0;
            Q_FOREACH(DB::ResultId id, model()->imageList(ViewOrder)) {
                maxCatsInText = qMax( noOfCategoriesForImage(id), maxCatsInText);
            }
        }
        h += charHeight * ( maxCatsInText ) +5;
    }
    return h;
}
