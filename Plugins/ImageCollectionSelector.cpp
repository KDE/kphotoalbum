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
#include <config-kpa-kipi.h>
#ifdef HASKIPI
#include "Plugins/ImageCollectionSelector.h"

Plugins::ImageCollectionSelector::ImageCollectionSelector( QWidget *parent, Interface *interface )
    : KIPI::ImageCollectionSelector( parent )
{
    _interface = interface;
    firstTimeVisible = true;
}

QList<KIPI::ImageCollection> Plugins::ImageCollectionSelector::selectedImageCollections() const
{
    if ( _interface ) {
        KIPI::ImageCollection collection = _interface->currentSelection();
        if (!collection.isValid()) {
            collection = _interface->currentAlbum();
        }
        if (collection.isValid()) {
            QList<KIPI::ImageCollection> res;
            res.append(collection);
            return res;
        }
        // probably never happens:
        return _interface->allAlbums();
    }
    return QList<KIPI::ImageCollection>();
}

void Plugins::ImageCollectionSelector::showEvent(QShowEvent *event) {
    KIPI::ImageCollectionSelector::showEvent(event);
    if (firstTimeVisible) {
        // fake one selection change to make HTML Export Plugin believe there really is a selection:
        emit selectionChanged();
        firstTimeVisible = false;
    }
}

#endif // HASKIPI
// vi:expandtab:tabstop=4 shiftwidth=4:
