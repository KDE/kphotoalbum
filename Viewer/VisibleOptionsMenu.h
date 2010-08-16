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
#include <QMap>
#include <QMenu>
class KToggleAction;
class KActionCollection;

namespace Viewer {

class VisibleOptionsMenu :public QMenu
{
Q_OBJECT
public:
    VisibleOptionsMenu( QWidget* parent, KActionCollection* actions);

signals:
    void visibleOptionsChanged();

private slots:
    void updateState();
    void toggleShowInfoBox( bool );
    void toggleShowCategory( bool );
    void toggleShowLabel( bool );
    void toggleShowDescription( bool );
    void toggleShowDate( bool );
    void toggleShowTime( bool );
    void toggleShowFilename( bool );
    void toggleShowEXIF( bool );
    void toggleShowImageSize( bool );

private:
    KToggleAction* _showInfoBox;
    KToggleAction* _showLabel;
    KToggleAction* _showDescription;
    KToggleAction* _showDate;
    KToggleAction* _showTime;
    KToggleAction* _showFileName;
    KToggleAction* _showExif;
    KToggleAction* _showImageSize;
    QList<KToggleAction*> _actionList;
};

}

#endif /* VISIBLEOPTIONSMENU_H */

