/* Copyright 2012 Jesper K. Pedersen <blackie@kde.org>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAINWINDOW_MERGETOOLTIP_H
#define MAINWINDOW_MERGETOOLTIP_H

#include "Utilities/ToolTip.h"

namespace MainWindow
{

class MergeToolTip : public Utilities::ToolTip
{
    Q_OBJECT

public:
    static MergeToolTip *instance();
    static void destroy();

protected:
    void placeWindow() override;

private:
    static MergeToolTip *s_instance;
    explicit MergeToolTip(QWidget *parent = nullptr);
};

} // namespace MainWindow

#endif // MAINWINDOW_MERGETOOLTIP_H
// vi:expandtab:tabstop=4 shiftwidth=4:
