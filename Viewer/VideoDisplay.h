/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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

#include "Display.h"
#include <kparts/componentfactory.h>
#include <QResizeEvent>
#include <Phonon/MediaObject>
namespace Phonon {
   class VideoWidget;
   class SeekSlider;
}

namespace Viewer
{

class VideoDisplay :public Viewer::Display
{
    Q_OBJECT

public:
    VideoDisplay( QWidget* parent );
    ~VideoDisplay();
    virtual bool setImage( DB::ImageInfoPtr info, bool forward );
    bool isPaused() const;
    bool isPlaying() const;

signals:
    void stopped();

public slots:
    void zoomIn();
    void zoomOut();
    void zoomFull();
    void zoomPixelForPixel();
    void stop();
    void playPause();
    void restart();
    void seek();

private slots:
    void phononStateChanged(Phonon::State, Phonon::State);

protected:
    void resize( double factor );
    OVERRIDE void resizeEvent( QResizeEvent* );
    void setup();
    void setVideoWidgetSize();

    enum ZoomType { FullZoom, PixelForPixelZoom, FixedZoom };

private:
    Phonon::MediaObject* _mediaObject;
    Phonon::VideoWidget* _videoWidget;
    Phonon::SeekSlider* _slider;
    ZoomType _zoomType;
    double _zoomFactor;
};

}

#endif /* VIEWER_VIDEODISPLAY_H */

