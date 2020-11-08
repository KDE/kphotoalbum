/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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

    enum ZoomType { FullZoom,
                    PixelForPixelZoom,
                    FixedZoom };

private:
    Phonon::MediaObject *m_mediaObject;
    Phonon::VideoWidget *m_videoWidget;
    Phonon::SeekSlider *m_slider;
    ZoomType m_zoomType;
    double m_zoomFactor;
};

}

#endif /* VIEWER_VIDEODISPLAY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
