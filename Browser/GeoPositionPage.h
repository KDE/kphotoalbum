// SPDX-FileCopyrightText: 2003-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

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

public Q_SLOTS:
    void slotNewRegionSelected(Map::GeoCoordinates::LatLonBox coordinates);

private:
    bool m_active;
};

}

#endif /* GEOPOSITIONPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
