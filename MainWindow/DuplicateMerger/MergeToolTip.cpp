/* SPDX-FileCopyrightText: 2012-2018 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

#include "moc_MergeToolTip.cpp"
