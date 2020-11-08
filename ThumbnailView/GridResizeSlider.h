/* SPDX-FileCopyrightText: 2015-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef GRIDRESIZESLIDER_H
#define GRIDRESIZESLIDER_H

#include "ThumbnailComponent.h"

#include <QSlider>
class QTimer;

namespace ThumbnailView
{
class ThumbnailWidget;

/**
 * @brief The GridResizeSlider class
 * The GridResizeSlider is a QSlider that ties into the ThumbnailView infrastructure,
 * as well as into the SettingsData on thumbnail size.
 *
 * Moving the slider changes the displayed thumbnail size, and changing the
 * thumbnail size in the SettingsData reflects on the slider.
 * Changes in SettingsData::thumbnailSize() are reflected in the maximum slider value.
 */
class GridResizeSlider : public QSlider, private ThumbnailComponent
{
    Q_OBJECT
public:
    explicit GridResizeSlider(ThumbnailFactory *factory);
    ~GridResizeSlider() override;

signals:
    void isResizing(bool);

protected:
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;

private slots:
    void enterGridResizingMode();
    void leaveGridResizingMode();
    void setCellSize(int size);
    void setMaximum(int size);

private:
    bool m_resizing;
    QTimer *m_timer;
};
}

#endif /* GRIDRESIZESLIDER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
