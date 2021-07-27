/* SPDX-FileCopyrightText: 2021 The KPhotoAlbum Development Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "VideoToolBar.h"

namespace QtAV
{
class VideoPreviewWidget;
class AVPlayer;
}

namespace Viewer
{

class QtAVVideoToolBar : public VideoToolBar
{
    Q_OBJECT

public:
    explicit QtAVVideoToolBar(QtAV::AVPlayer *player, QWidget *parent);
    void closePreview() override;

protected:
    void onTimeSliderHover(const QPoint &pos, int value) override;

private:
    void slotVideoStarted();

    QtAV::AVPlayer *m_player = nullptr;
    QtAV::VideoPreviewWidget *m_preview = nullptr;
};

} // namespace Viewer
