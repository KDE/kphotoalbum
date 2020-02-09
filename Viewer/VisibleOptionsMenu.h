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
#ifndef VISIBLEOPTIONSMENU_H
#define VISIBLEOPTIONSMENU_H

#include <QList>
#include <QMenu>
class KToggleAction;
class KActionCollection;

namespace Viewer
{

/**
 * @brief The VisibleOptionsMenu lets the user choose what kind of information is shown in the InfoBox.
 * It is used in the context menu of the viewer.
 *
 * If any option is changed, visibleOptionsChanged() is emitted.
 * Changes are written directly to the global application settings.
 */
class VisibleOptionsMenu : public QMenu
{
    Q_OBJECT
public:
    VisibleOptionsMenu(QWidget *parent, KActionCollection *actions);

signals:
    void visibleOptionsChanged();

private slots:
    void updateState();
    void toggleShowInfoBox(bool);
    void toggleShowCategory(bool);
    void toggleShowLabel(bool);
    void toggleShowDescription(bool);
    void toggleShowDate(bool);
    void toggleShowTime(bool);
    void toggleShowFilename(bool);
    void toggleShowEXIF(bool);
    void toggleShowImageSize(bool);
    void toggleShowRating(bool);

private:
    KToggleAction *m_showInfoBox;
    KToggleAction *m_showLabel;
    KToggleAction *m_showDescription;
    KToggleAction *m_showDate;
    KToggleAction *m_showTime;
    KToggleAction *m_showFileName;
    KToggleAction *m_showExif;
    KToggleAction *m_showImageSize;
    KToggleAction *m_showRating;
    QList<KToggleAction *> m_actionList;
};

}

#endif /* VISIBLEOPTIONSMENU_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
