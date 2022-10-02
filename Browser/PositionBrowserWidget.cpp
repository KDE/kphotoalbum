// SPDX-FileCopyrightText: 2016-2017 Matthias Füssel <matthias.fuessel@gmx.net>
// SPDX-FileCopyrightText: 2016-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "PositionBrowserWidget.h"

#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <MainWindow/Logging.h>

#include <KLocalizedString>
#include <QElapsedTimer>
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
    QElapsedTimer timer;
    timer.start();
    m_mapView->displayStatus(Map::MapView::MapStatus::Loading);
    m_mapView->clear();
    DB::FileNameList images = DB::ImageDB::instance()->search(searchInfo);
    int count = 0;
    for (DB::FileNameList::const_iterator imageIter = images.constBegin(); imageIter < images.constEnd(); ++imageIter) {
        DB::ImageInfoPtr image = imageIter->info();
        if (image->coordinates().hasCoordinates()) {
            count++;
            m_mapView->addImage(image);
        }
    }
    m_mapView->displayStatus(Map::MapView::MapStatus::SearchCoordinates);
    m_mapView->zoomToMarkers();
    qCDebug(TimingLog) << "Browser::PositionBrowserWidget::showImages(): added" << count << "images in" << timer.elapsed() << "ms.";
}

void Browser::PositionBrowserWidget::clearImages()
{
    m_mapView->clear();
}

void Browser::PositionBrowserWidget::slotRegionSelectionChanged()
{
    if (m_mapView->regionSelected()) {
        Q_EMIT signalNewRegionSelected(m_mapView->getRegionSelection());
    }
}
