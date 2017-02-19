/* Copyright (C) 2016-2017 Matthias Füssel <matthias.fuessel@gmx.net>

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

#ifndef POSITIONBROWSERWIDGET_H_
#define POSITIONBROWSERWIDGET_H_

#include "qwidget.h"
#include "DB/FileNameList.h"
#include "DB/ImageSearchInfo.h"
#include "Map/MapView.h"

namespace Browser {

class PositionBrowserWidget : public QWidget {
    Q_OBJECT

public:
    PositionBrowserWidget( QWidget* parent );
    virtual ~PositionBrowserWidget();
    virtual void showImages( const DB::ImageSearchInfo& searchInfo );
    virtual void clearImages();

Q_SIGNALS:
    void signalNewRegionSelected(KGeoMap::GeoCoordinates::Pair coordinates);

public slots:
    void slotRegionSelectionChanged();

private:
    Map::MapView *m_mapView;
};

}

#endif /* POSITIONBROWSERWIDGET_H_ */
