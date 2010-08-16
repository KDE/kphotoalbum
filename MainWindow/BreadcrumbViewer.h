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
#ifndef BREADCRUMBVIEWER_H
#define BREADCRUMBVIEWER_H
#include <QLabel>
#include <Browser/BreadcrumbList.h>

class BreadcrumbViewer :public QLabel
{
    Q_OBJECT

public:
    BreadcrumbViewer();
    OVERRIDE QSize minimumSizeHint() const;

public slots:
    void setBreadcrumbs( const Browser::BreadcrumbList& list );

signals:
    void widenToBreadcrumb( const Browser::Breadcrumb& );

protected:
    OVERRIDE void resizeEvent( QResizeEvent* event );

private slots:
    void linkClicked( const QString& link );

private:
    void updateText();

private:
    Browser::BreadcrumbList _activeCrumbs;
};

#endif /* BREADCRUMBVIEWER_H */

