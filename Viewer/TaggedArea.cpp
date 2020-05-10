/* Copyright (C) 2014-2020 The KPhotoAlbum Development Team

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

#include <KLocalizedString>
#include <QDebug>
#include <QStyle>

Viewer::TaggedArea::TaggedArea(QWidget *parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::Box);
}

void Viewer::TaggedArea::setTagInfo(QString category, QString localizedCategory, QString tag)
{
    setToolTip(tag + QString::fromLatin1(" (") + localizedCategory + QString::fromLatin1(")"));
    m_tagInfo = QPair<QString, QString>(category, tag);
}

void Viewer::TaggedArea::setActualGeometry(QRect geometry)
{
    m_actualGeometry = geometry;
}

QRect Viewer::TaggedArea::actualGeometry() const
{
    return m_actualGeometry;
}

void Viewer::TaggedArea::setSelected(bool selected)
{
    m_selected = selected;
    repolish();
}

bool Viewer::TaggedArea::selected() const
{
    return m_selected;
}

void Viewer::TaggedArea::deselect()
{
    setSelected(false);
}

void Viewer::TaggedArea::checkIsSelected(const QPair<QString, QString> &tagData)
{
    setSelected(tagData == m_tagInfo);
}

void Viewer::TaggedArea::repolish()
{
    style()->unpolish(this);
    style()->polish(this);
    update();
}

bool Viewer::TaggedArea::highlighted() const
{
    return m_highlighted;
}

void Viewer::TaggedArea::setHighlighted(bool highlighted)
{
    m_highlighted = highlighted;
    repolish();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
