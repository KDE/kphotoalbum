/*
   Copyright (C) 2018 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
   Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef MEDIACOUNT_H
#define MEDIACOUNT_H

namespace DB
{
class MediaCount
{
public:
    MediaCount()
        : m_null(true)
        , m_images(0)
        , m_videos(0)
    {
    }
    MediaCount(int images, int videos)
        : m_null(false)
        , m_images(images)
        , m_videos(videos)
    {
    }
    bool isNull() const { return m_null; }
    int images() const { return m_images; }
    int videos() const { return m_videos; }
    int total() const { return m_images + m_videos; }

private:
    bool m_null;
    int m_images;
    int m_videos;
};

}

#endif /* MEDIACOUNT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
