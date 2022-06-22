// SPDX-FileCopyrightText: 2003-2021 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "AbstractDisplay.h"
#include "VideoDisplay.h"

#include <QResizeEvent>
#include <phonon/mediaobject.h>

namespace Phonon
{
class VideoWidget;
class AudioOutput;
}

namespace Viewer
{

class PhononDisplay : public Viewer::VideoDisplay
{
    Q_OBJECT

public:
    explicit PhononDisplay(QWidget *parent);
    ~PhononDisplay() override;
    bool setImageImpl(DB::ImageInfoPtr info, bool forward) override;
    bool isPaused() const override;
    bool isPlaying() const override;
    QImage screenShoot() override;
    void relativeSeek(int msec) override;

signals:
    void stopped();

public slots:
    void zoomIn() override;
    void zoomOut() override;
    void zoomFull() override;
    void zoomPixelForPixel() override;
    void stop() override;
    void playPause() override;
    void restart() override;
    void rotate(const DB::ImageInfoPtr &info) override;
    void changeVolume(int newVolumePercent);

private slots:
    void phononStateChanged(Phonon::State, Phonon::State);
    void updateVolume(qreal newVolumeVolt);

protected:
    void resize(double factor);
    void resizeEvent(QResizeEvent *) override;
    void setup();
    void setVideoWidgetSize();

    enum ZoomType { FullZoom,
                    PixelForPixelZoom,
                    FixedZoom };

private:
    Phonon::MediaObject *m_mediaObject = nullptr;
    Phonon::VideoWidget *m_videoWidget = nullptr;
    Phonon::AudioOutput *m_audioDevice = nullptr;
    class VideoToolBar *m_videoToolBar = nullptr;
    ZoomType m_zoomType = FullZoom;
    double m_zoomFactor = 1;
};

}

// vi:expandtab:tabstop=4 shiftwidth=4:
