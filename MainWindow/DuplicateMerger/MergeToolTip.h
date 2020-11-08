/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>
  
   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MAINWINDOW_MERGETOOLTIP_H
#define MAINWINDOW_MERGETOOLTIP_H

#include <Utilities/ToolTip.h>

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
