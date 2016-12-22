/*
 * PositionBrowserWidget.h
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
