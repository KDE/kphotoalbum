/* Copyright (C) 2003-2019 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef VIEWER_VIDEODISPLAY_H
#define VIEWER_VIDEODISPLAY_H

#include "AbstractDisplay.h"

#include <QResizeEvent>
#include <phonon/mediaobject.h>
namespace Phonon
{
class VideoWidget;
class SeekSlider;
}

namespace Viewer
{

class VideoDisplay : public Viewer::AbstractDisplay
{
    Q_OBJECT

public:
    explicit VideoDisplay(QWidget *parent);
    ~VideoDisplay() override;
    bool setImage(DB::ImageInfoPtr info, bool forward) override;
    bool isPaused() const;
    bool isPlaying() const;
    QImage screenShoot();

signals:
    void stopped();

public slots:
    void zoomIn() override;
    void zoomOut() override;
    void zoomFull() override;
    void zoomPixelForPixel() override;
    void stop();
    void playPause();
    void restart();
    void seek();

private slots:
    void phononStateChanged(Phonon::State, Phonon::State);

protected:
    void resize(double factor);
    void resizeEvent(QResizeEvent *) override;
    void setup();
    void setVideoWidgetSize();
    void setupRemoteDisplayInfo();

    enum ZoomType { FullZoom,
                    PixelForPixelZoom,
                    FixedZoom };

private:
    Phonon::MediaObject *m_mediaObject;
    Phonon::VideoWidget *m_videoWidget;
    Phonon::SeekSlider *m_slider;
    ZoomType m_zoomType;
    double m_zoomFactor;
    bool m_showNextVideoLocally = false;
};

}

#endif /* VIEWER_VIDEODISPLAY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
