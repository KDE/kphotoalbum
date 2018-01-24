/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef GEOPOSITIONPAGE_H
#define GEOPOSITIONPAGE_H
#include "BrowserPage.h"

#include <DB/Category.h>
#include <DB/CategoryPtr.h>
#include <DB/ImageSearchInfo.h>

class QAbstractItemModel;
class FlatCategoryModel;
class BrowserWidget;

namespace Browser
{

/**
 * \brief The Browser page for categories.
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 */
class GeoPositionPage : public BrowserPage
{
    Q_OBJECT
public:
    GeoPositionPage(const DB::ImageSearchInfo &info, BrowserWidget *browser);
    Viewer viewer() override;
    void activate() override;
    void deactivate() override;
    bool isSearchable() const override;
    bool showDuringMovement() const override;

public slots:
    void slotNewRegionSelected(Map::GeoCoordinates::Pair coordinates);

private:
    bool active;
};

}

#endif /* GEOPOSITIONPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
