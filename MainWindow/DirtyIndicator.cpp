/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include "DirtyIndicator.h"
#include <QLabel>
#include <QPixmap>
#include <kiconloader.h>

static MainWindow::DirtyIndicator *s_instance = nullptr;
bool MainWindow::DirtyIndicator::s_autoSaveDirty = false;
bool MainWindow::DirtyIndicator::s_saveDirty = false;
bool MainWindow::DirtyIndicator::s_suppressMarkDirty = false;

MainWindow::DirtyIndicator::DirtyIndicator(QWidget *parent)
    : QLabel(parent)
{
    m_dirtyPix = QPixmap(SmallIcon(QString::fromLatin1("media-floppy")));
    setFixedWidth(m_dirtyPix.width() + 10);
    s_instance = this;

    // Might have been marked dirty even before the indicator had been created, by the database searching during loading.
    if (s_saveDirty)
        markDirty();
}

void MainWindow::DirtyIndicator::suppressMarkDirty(bool state)
{
    MainWindow::DirtyIndicator::s_suppressMarkDirty = state;
}

void MainWindow::DirtyIndicator::markDirty()
{
    if (MainWindow::DirtyIndicator::s_suppressMarkDirty) {
        return;
    }

    if (s_instance) {
        s_instance->markDirtySlot();
    } else {
        s_saveDirty = true;
        s_autoSaveDirty = true;
    }
}

void MainWindow::DirtyIndicator::markDirtySlot()
{
    if (MainWindow::DirtyIndicator::s_suppressMarkDirty) {
        return;
    }

    s_saveDirty = true;
    s_autoSaveDirty = true;
    setPixmap(m_dirtyPix);
    emit dirty();
}

void MainWindow::DirtyIndicator::autoSaved()
{
    s_autoSaveDirty = false;
}

void MainWindow::DirtyIndicator::saved()
{
    s_autoSaveDirty = false;
    s_saveDirty = false;
    setPixmap(QPixmap());
}

bool MainWindow::DirtyIndicator::isSaveDirty() const
{
    return s_saveDirty;
}

bool MainWindow::DirtyIndicator::isAutoSaveDirty() const
{
    return s_autoSaveDirty;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
