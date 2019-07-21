/* Copyright (C) 2016-2019 The KPhotoAlbum Development Team
   Copyright (C) 2016-2017 Matthias Füssel <matthias.fuessel@gmx.net>

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
#include "PositionBrowserWidget.h"

#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>

#include <KLocalizedString>
#include <QProgressBar>
#include <QVBoxLayout>
#include <qdom.h>
#include <qlabel.h>
#include <qurl.h>

Browser::PositionBrowserWidget::PositionBrowserWidget(QWidget *parent)
    : QWidget(parent)
{
    m_mapView = new Map::MapView(this);
    m_mapView->displayStatus(Map::MapView::MapStatus::Loading);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_mapView);
    connect(m_mapView, &Map::MapView::signalRegionSelectionChanged,
            this, &Browser::PositionBrowserWidget::slotRegionSelectionChanged);
}

Browser::PositionBrowserWidget::~PositionBrowserWidget()
{
}

void Browser::PositionBrowserWidget::showImages(const DB::ImageSearchInfo &searchInfo)
{
    m_mapView->displayStatus(Map::MapView::MapStatus::Loading);
    m_mapView->clear();
    DB::FileNameList images = DB::ImageDB::instance()->search(searchInfo);
    for (DB::FileNameList::const_iterator imageIter = images.constBegin(); imageIter < images.constEnd(); ++imageIter) {
        DB::ImageInfoPtr image = imageIter->info();
        if (image->coordinates().hasCoordinates()) {
            m_mapView->addImage(image);
        }
    }
    m_mapView->displayStatus(Map::MapView::MapStatus::SearchCoordinates);
    m_mapView->zoomToMarkers();
}

void Browser::PositionBrowserWidget::clearImages()
{
    m_mapView->clear();
}

void Browser::PositionBrowserWidget::slotRegionSelectionChanged()
{
    if (m_mapView->regionSelected()) {
        emit signalNewRegionSelected(m_mapView->getRegionSelection());
    }
}
