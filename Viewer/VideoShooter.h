// SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIDEOSHOOTER_H
#define VIDEOSHOOTER_H

#include <DB/ImageInfoPtr.h>

#include <QObject>

namespace Viewer
{
class ViewerWidget;

class VideoShooter : public QObject
{
    Q_OBJECT

public:
    static void go(const DB::ImageInfoPtr &info, Viewer::ViewerWidget *viewer);

private slots:
    void start(const DB::ImageInfoPtr &info, ViewerWidget *);
    void doShoot();

private:
    static VideoShooter *s_instance;
    explicit VideoShooter();
    ViewerWidget *m_viewer = nullptr;
    bool m_infoboxVisible = false;
    DB::ImageInfoPtr m_info;
    bool m_wasPlaying = false;
};

}
#endif // VIDEOSHOOTER_H
// vi:expandtab:tabstop=4 shiftwidth=4:
