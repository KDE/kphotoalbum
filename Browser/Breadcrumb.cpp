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

#include "Breadcrumb.h"
#include <KLocalizedString>

int Browser::Breadcrumb::s_count = 0;

Browser::Breadcrumb::Breadcrumb(const QString &text, bool isBeginning)
    : m_index(++s_count)
    , m_isBeginning(isBeginning)
    , m_isView(false)
    , m_text(text)
{
}

Browser::Breadcrumb Browser::Breadcrumb::empty()
{
    return Breadcrumb(QString());
}

Browser::Breadcrumb Browser::Breadcrumb::home()
{
    return Breadcrumb(i18nc("As in 'all pictures'.", "All"), true);
}

QString Browser::Breadcrumb::text() const
{
    return m_text;
}

bool Browser::Breadcrumb::isBeginning() const
{
    return m_isBeginning;
}

bool Browser::Breadcrumb::operator==(const Breadcrumb &other) const
{
    return other.m_index == m_index;
}

bool Browser::Breadcrumb::operator!=(const Breadcrumb &other) const
{
    return !(other == *this);
}

Browser::Breadcrumb Browser::Breadcrumb::view()
{
    Breadcrumb res(QString(), false);
    res.m_isView = true;
    return res;
}

bool Browser::Breadcrumb::isView() const
{
    return m_isView;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
