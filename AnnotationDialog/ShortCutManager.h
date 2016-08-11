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
#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QSet>
#include <QList>
#include <QPair>

class QDockWidget;
class QLabel;
class QWidget;

namespace AnnotationDialog {

typedef QPair<QDockWidget*, QWidget*> DockPair;

/**
 * This class is to help set up the right shortcuts for the annotation dialog.
 * The are two problems with KDE's default shortcuts:
 * (1) There is a bug in Qt, so that shortcuts are actually not set up for
 * QDockWidgets. (This might be fixed when you read this, of course)
 * (2) KDE's shortcut handling does of course not know anything about the
 * annotation dialog, and may therefore not set up an optiomal set of
 * shortcuts.
 * This class ensures that each of the list selects (which are what the
 * user will use the most) will get shortcuts assigned first, and then the
 * other widgets will.
 */
class ShortCutManager
{
public:
    void addDock( QDockWidget* dock, QWidget* buddy );
    void addLabel( QLabel* label );
    void setupShortCuts();
    void addTaken( const QString& );

private:
    QList< DockPair > m_docks;
    QList< QLabel* > m_labelWidgets;
    QSet<QChar> m_taken;
};

}

#endif /* SHORTCUTMANAGER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
