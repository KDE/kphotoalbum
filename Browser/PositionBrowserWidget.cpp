/*
 * PositionBrowserWidget.cpp
 */

#include "PositionBrowserWidget.h"

#include <QProgressBar>
#include <QVBoxLayout>
#include <qdom.h>
#include <qurl.h>
#include <qlabel.h>

#include <KLocalizedString>

#include "DB/ImageInfo.h"
#include "DB/ImageDB.h"

namespace Browser {

//ImageSelector::ImageSelector()
//{
//}
//
//ImageSelector::~ImageSelector()
//{
//}

PositionBrowserWidget::PositionBrowserWidget( QWidget* parent )
    :QWidget(parent)
{
    m_mapView = new Map::MapView(this);
    m_mapView->displayStatus(Map::MapView::MapStatus::Loading);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_mapView);
    connect(m_mapView, SIGNAL(signalRegionSelectionChanged()), this, SLOT(slotRegionSelectionChanged()));
}

PositionBrowserWidget::~PositionBrowserWidget()
{
}

void PositionBrowserWidget::showImages( const DB::ImageSearchInfo& searchInfo )
{
    m_mapView->displayStatus(Map::MapView::MapStatus::Loading);
    m_mapView->clear();
    DB::FileNameList images = DB::ImageDB::instance()->search(searchInfo);
    for (DB::FileNameList::const_iterator imageIter = images.begin(); imageIter < images.end(); ++imageIter) {
        DB::ImageInfoPtr image = imageIter->info();
        if (image->coordinates().hasCoordinates()) {
            m_mapView->addImage(image);
        }
    }
    m_mapView->displayStatus(Map::MapView::MapStatus::SearchCoordinates);
    m_mapView->zoomToMarkers();
}

void PositionBrowserWidget::clearImages()
{
    m_mapView->clear();
}

void PositionBrowserWidget::slotRegionSelectionChanged()
{
    if (m_mapView->regionSelected()) {
        emit signalNewRegionSelected(m_mapView->getRegionSelection());
    }
}


}
