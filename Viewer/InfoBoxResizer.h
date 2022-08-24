// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

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
    bool m_left = false;
    bool m_right = false;
    bool m_top = false;
    bool m_bottom = false;
    bool m_active = false;
};

}

#endif /* INFOBOXRESIZER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
