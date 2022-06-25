/* SPDX-FileCopyrightText: 2014-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
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

#include "moc_TaggedArea.cpp"
