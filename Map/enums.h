// SPDX-FileCopyrightText: 2019-2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MAP_ENUMS_H
#define MAP_ENUMS_H

namespace Map
{

/**
 * @brief The MapStyle enum influences the drawing style of images/clusters on the MapView.
 */
enum class MapStyle {
    ShowPins, ///< Show GeoClusters by default and show pins instead of thumbnails when zooming in.
    ShowThumbnails, ///< Show GeoClusters by default and show thumbnails when zooming in.
    ForceShowThumbnails, ///< Show individual thumbnails even for GeoClusters
};

}

#endif /* MAP_ENUMS_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
