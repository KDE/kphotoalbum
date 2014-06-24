/* Copyright (C) 2014 Tobias Leupold <tobias.leupold@web.de>

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
#include "TaggedArea.h"
#include <klocale.h>
#include <QApplication>

Viewer::TaggedArea::TaggedArea(QWidget *parent) : QFrame(parent)
{
    setFrameShape(QFrame::Box);
    resetViewStyle();
}

Viewer::TaggedArea::~TaggedArea()
{
}

void Viewer::TaggedArea::setTagInfo(QString category, QString localizedCategory, QString tag)
{
    setToolTip(tag + QString::fromLatin1(" (") + localizedCategory + QString::fromLatin1(")"));
    _tagInfo = QPair<QString, QString>(category, tag);
}

void Viewer::TaggedArea::setActualGeometry(QRect geometry)
{
    _actualGeometry = geometry;
}

QRect Viewer::TaggedArea::actualGeometry() const
{
    return _actualGeometry;
}

void Viewer::TaggedArea::resetViewStyle()
{
    setStyleSheet(QString::fromLatin1(
        "Viewer--TaggedArea { border: none; background-color: none; }"
        "Viewer--TaggedArea:hover { border: 1px solid rgb(0,255,0,99); background-color: rgb(255,255,255,30); }"
    ));
}

void Viewer::TaggedArea::checkShowArea(QPair<QString, QString> tagData)
{
    if (tagData == _tagInfo) {
        setStyleSheet(QString::fromLatin1("Viewer--TaggedArea { border: 1px solid rgb(0,255,0,99); background-color: rgb(255,255,255,30); }"));
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
