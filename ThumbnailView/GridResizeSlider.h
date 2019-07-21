/* Copyright (C) 2015 Johannes Zarl <johannes@zarl.at>

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
 * Moving the slider changes the actual thumbnail size, and changing the actual
 * thumbnail size in the SettingsData reflects on the slider.
 * Changes in SettingsData::thumbnailSize() are reflected in the maximum slider value.
 */
class GridResizeSlider : public QSlider, private ThumbnailComponent
{
    Q_OBJECT
public:
    explicit GridResizeSlider(ThumbnailFactory *factory);
    ~GridResizeSlider() override;

public slots:
    void increaseThumbnailSize();
    void decreaseThumbnailSize();

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
    void calculateNewThumbnailSize(int perRowDifference);
};
}

#endif /* GRIDRESIZESLIDER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
