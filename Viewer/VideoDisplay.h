// SPDX-FileCopyrightText: 2003-2021 Jesper K. Pedersen <blackie@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIDEODISPLAY_H
#define VIDEODISPLAY_H

#include "AbstractDisplay.h"

namespace Viewer
{
class VideoDisplay : public Viewer::AbstractDisplay
{
    Q_OBJECT
public:
    using Viewer::AbstractDisplay::AbstractDisplay;
    virtual bool isPaused() const = 0;
    virtual void playPause() = 0;
    virtual bool isPlaying() const = 0;
    virtual QImage screenShoot() = 0;
    virtual void relativeSeek(int msec) = 0;
    virtual void restart() = 0;

signals:
    void stopped();
};
}
#endif // VIDEODISPLAY_H
