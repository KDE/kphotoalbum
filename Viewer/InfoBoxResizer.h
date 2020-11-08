/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
