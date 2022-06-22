// SPDX-FileCopyrightText: 2021 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QWidget>
class Slider;
class QLabel;
class QToolButton;
class QSlider;

namespace Viewer
{

class VideoToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit VideoToolBar(QWidget *parent = nullptr);
    void setRange(int min, int max);
    int maximum() const;

    void setPosition(int value);

    void setVolume(int volume);
    bool isMuted() const;
    void setMuted(bool b);

    virtual void closePreview() { }

protected:
    virtual void onTimeSliderHover(const QPoint &pos, int value);
    void setSeekSliderEnabled(bool b);

signals:
    void positionChanged(int value);
    void volumeChanged(int volume);
    void muted(bool muted);

private:
    QString format() const;

    QLabel *m_currentOffset = nullptr;
    Slider *m_offsetSlider = nullptr;
    QLabel *m_totalTime = nullptr;
    QToolButton *m_muteButton = nullptr;
    QSlider *m_volumeSlider = nullptr;
    QLabel *m_percentageLabel = nullptr;
    bool m_isMuted = false;
};

} // namespace Viewer
