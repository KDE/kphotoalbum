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
#ifndef MYIMAGECOLLECTIONSELECTOR_H
#define MYIMAGECOLLECTIONSELECTOR_H

#include "Plugins/ImageCollection.h"
#include "Plugins/Interface.h"
#include <config-kpa-kipi.h>

namespace Plugins
{
/** This class should provide a widget for selecting one ore more image collection for plugins that want the user to
 *  select images.
 *
 * Since selecting images is all kphotoalbum is about ;-), this implementation just passes the images that are (or
 * would be) currently visible in thumbnail view - if some of them are selected, only selected ones, otherwise all.
 *
 * The widget shown is currently empty.
 *
 * Possible improvements:
 *  * show some description of the currently selected images instead of just nothing
 *  * give the user the possibility to group the selected images into image collections by some category: this would be
 *    useful as i.e. html export plugin uses the names of image collections as headlines and groups the images visually by
 *    image collection.
 */
class ImageCollectionSelector : public KIPI::ImageCollectionSelector
{
public:
    ImageCollectionSelector(QWidget *parent, Interface *interface);
    QList<KIPI::ImageCollection> selectedImageCollections() const override;

protected:
    // just fake a selectionChanged event when first shown to make export plugin happy:
    void showEvent(QShowEvent *event) override;

private:
    Interface *m_interface;
    bool m_firstTimeVisible;
};

}

#endif /* MYIMAGECOLLECTIONSELECTOR_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
