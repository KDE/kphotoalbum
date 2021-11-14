// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoDisplay.h"

#include <DB/ImageInfo.h>

Viewer::DummyVideoDisplay::DummyVideoDisplay(QWidget *parent)
    : Viewer::VideoDisplay(parent)
{
}

bool Viewer::DummyVideoDisplay::isPaused() const
{
    return m_paused;
}

void Viewer::DummyVideoDisplay::playPause()
{
    m_paused = !m_paused;
}

bool Viewer::DummyVideoDisplay::isPlaying() const
{
    return m_playing;
}

QImage Viewer::DummyVideoDisplay::screenShoot()
{
    return {};
}

void Viewer::DummyVideoDisplay::relativeSeek(int) { }

void Viewer::DummyVideoDisplay::restart()
{
    m_paused = false;
    m_playing = true;
}

void Viewer::DummyVideoDisplay::stop()
{
    m_paused = false;
    m_playing = false;
}

void Viewer::DummyVideoDisplay::rotate(const DB::ImageInfoPtr &) { }

bool Viewer::DummyVideoDisplay::setImageImpl(DB::ImageInfoPtr, bool)
{
    m_playing = true;
    return true;
}
