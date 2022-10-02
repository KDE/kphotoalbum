// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IMAGEDISPLAY_H
#define IMAGEDISPLAY_H

#include "AbstractDisplay.h"

#include <DB/ImageInfoPtr.h>
#include <ImageManager/ImageClientInterface.h>
#include <kpabase/FileNameList.h>
#include <kpabase/SettingsData.h>

#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <qimage.h>
#include <qpixmap.h>
class QTimer;

namespace DB
{
class ImageInfo;
}

namespace Viewer
{
class ViewHandler;
class ViewerWidget;

struct ViewPreloadInfo {
    ViewPreloadInfo() { }
    ViewPreloadInfo(const QImage &img, const QSize &size, int angle)
        : img(img)
        , size(size)
        , angle(angle)
    {
    }
    QImage img;
    QSize size;
    int angle = 0;
};

class ImageDisplay : public Viewer::AbstractDisplay, public ImageManager::ImageClientInterface
{
    Q_OBJECT
public:
    explicit ImageDisplay(QWidget *parent);
    QImage currentViewAsThumbnail() const;
    void pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image) override;
    void setImageList(const DB::FileNameList &list);

    void filterNone();
    void filterSelected();
    bool filterMono();
    bool filterBW();
    bool filterContrastStretch();
    bool filterHistogramEqualization();

public Q_SLOTS:
    void zoomIn() override;
    void zoomOut() override;
    void zoomFull() override;
    void zoomPixelForPixel() override;
    void stop() override { }
    void rotate(const DB::ImageInfoPtr &info) override;

protected Q_SLOTS:
    bool setImageImpl(DB::ImageInfoPtr info, bool forward) override;
    void hideCursor();
    void showCursor();
    void disableCursorHiding();
    void enableCursorHiding();

Q_SIGNALS:
    void possibleChange();
    void imageReady();
    void setCaptionInfo(const QString &info);
    void viewGeometryChanged(QSize viewSize, QRect zoomWindow, double sizeRatio);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void hideEvent(QHideEvent *) override;
    QPoint mapPos(QPoint);
    QPoint offset(int logicalWidth, int logicalHeight, int physicalWidth, int physicalHeight, double *ratio);
    void xformPainter(QPainter *);
    void cropAndScale();
    void updatePreload();
    int indexOf(const DB::FileName &fileName);
    void requestImage(const DB::ImageInfoPtr &info, bool priority = false);

    /** display zoom factor in title of display window */
    void updateZoomCaption();

    friend class ViewHandler;
    void zoom(QPoint p1, QPoint p2);
    void normalize(QPoint &p1, QPoint &p2);
    void pan(const QPoint &);
    void busy();
    void unbusy();
    bool isImageZoomed(const Settings::StandardViewSize type, const QSize &imgSize);
    void updateZoomPoints(const Settings::StandardViewSize type, const QSize &imgSize);
    void potentiallyLoadFullSize();
    double sizeRatio(const QSize &baseSize, const QSize &newSize) const;

private:
    QImage m_loadedImage;
    QImage m_croppedAndScaledImg;

    ViewHandler *m_viewHandler;

    // zoom points in the coordinate system of the image.
    QPoint m_zStart;
    QPoint m_zEnd;

    QMap<int, ViewPreloadInfo> m_cache;
    DB::FileNameList m_imageList;
    QMap<QString, DB::ImageInfoPtr> m_loadMap;
    bool m_reloadImageInProgress;
    bool m_forward;
    int m_curIndex;
    bool m_busy;
    ViewerWidget *m_viewer;

    QTimer *m_cursorTimer;
    bool m_cursorHiding;
};
}

#endif /* IMAGEDISPLAY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
