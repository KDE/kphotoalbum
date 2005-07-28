/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef INFOBOX_H
#define INFOBOX_H
#include <qtextbrowser.h>
class QToolButton;
class Viewer;

class InfoBox :public QTextBrowser {
    Q_OBJECT

public:
    InfoBox( Viewer* parent, const char* name = 0 );
    void setInfo( const QString& text, const QMap<int, QPair<QString,QString> >& linkMap );
    virtual void setSource( const QString& which );
    void setSize();

protected slots:
    void jumpToContext();

protected:
    virtual void contentsMouseMoveEvent( QMouseEvent* );
    void showBrowser();

private:
    QMap<int, QPair<QString,QString> > _linkMap;
    Viewer* _viewer;
    QToolButton* _jumpToContext;
};


#endif /* INFOBOX_H */

