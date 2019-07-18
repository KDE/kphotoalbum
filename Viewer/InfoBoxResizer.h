/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef INFOBOXRESIZER_H
#define INFOBOXRESIZER_H

#include <QPoint>
namespace Viewer
{
class InfoBox;
}

namespace Viewer
{

class InfoBoxResizer
{
public:
    explicit InfoBoxResizer(Viewer::InfoBox *infoBox);
    void setup(bool left, bool right, bool top, bool bottom);
    void setPos(QPoint pos);
    void deactivate();
    bool isActive() const;

private:
    InfoBox *m_infoBox;
    bool m_left;
    bool m_right;
    bool m_top;
    bool m_bottom;
    bool m_active;
};

}

#endif /* INFOBOXRESIZER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
