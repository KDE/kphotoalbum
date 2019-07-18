/* Copyright 2012-2018 Jesper K. Pedersen <blackie@kde.org>
  
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

#include "MergeToolTip.h"

namespace MainWindow
{

MergeToolTip *MergeToolTip::s_instance = nullptr;

MainWindow::MergeToolTip *MainWindow::MergeToolTip::instance()
{
    if (!s_instance)
        s_instance = new MergeToolTip;
    return s_instance;
}

void MergeToolTip::destroy()
{
    delete s_instance;
    s_instance = nullptr;
}

MergeToolTip::MergeToolTip(QWidget *parent)
    : Utilities::ToolTip(parent, Qt::Window)
{
}

void MergeToolTip::placeWindow()
{
    raise();
}

} // namespace MainWindow

// vi:expandtab:tabstop=4 shiftwidth=4:
