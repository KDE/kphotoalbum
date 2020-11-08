/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
