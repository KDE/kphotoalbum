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
#ifndef DIRTYINDICATOR_H
#define DIRTYINDICATOR_H

#include <qlabel.h>
#include <qpixmap.h>

namespace MainWindow
{
class Window;

class DirtyIndicator : public QLabel
{
    Q_OBJECT

public:
    static void markDirty();
    static void suppressMarkDirty(bool state);

public slots:
    void markDirtySlot();

signals:
    void dirty();

private:
    friend class StatusBar;
    friend class Window;
    DirtyIndicator(QWidget *parent);
    void autoSaved();
    void saved();
    bool isSaveDirty() const;
    bool isAutoSaveDirty() const;

    QPixmap m_dirtyPix;
    static bool s_autoSaveDirty;
    static bool s_saveDirty;

    static bool s_suppressMarkDirty;
};

}

#endif /* DIRTYINDICATOR_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
