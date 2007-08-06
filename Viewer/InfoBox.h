/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include <QMouseEvent>
#include "ViewerWidget.h"
#include <QTextBrowser>
class QToolButton;

namespace Viewer
{
class InfoBox :public QTextBrowser {
    Q_OBJECT

public:
    InfoBox( ViewerWidget* parent );
    void setInfo( const QString& text, const QMap<int, QPair<QString,QString> >& linkMap );
    virtual void setSource( const QString& which );
    void setSize();

protected slots:
    void jumpToContext();
    void linkHovered(const QString&);

protected:
    virtual void mouseMoveEvent( QMouseEvent* );
    virtual void mousePressEvent( QMouseEvent* );
    virtual void mouseReleaseEvent( QMouseEvent* );

    void showBrowser();

private:
    QMap<int, QPair<QString,QString> > _linkMap;
    ViewerWidget* _viewer;
    QToolButton* _jumpToContext;
    bool _hoveringOverLink;
};

}


#endif /* INFOBOX_H */

