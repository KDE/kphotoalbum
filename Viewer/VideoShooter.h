/* Copyright (C) 2012 Jesper K. Pedersen <blackie@kde.org>

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
    ViewerWidget *m_viewer;
    bool m_infoboxVisible;
    DB::ImageInfoPtr m_info;
    bool m_wasPlaying;
};

}
#endif // VIDEOSHOOTER_H
// vi:expandtab:tabstop=4 shiftwidth=4:
