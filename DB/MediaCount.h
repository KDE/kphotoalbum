/*
   SPDX-FileCopyrightText: 2018 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
   SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
