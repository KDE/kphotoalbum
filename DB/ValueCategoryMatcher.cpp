/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "ValueCategoryMatcher.h"

#include "ImageDB.h"
#include "Logging.h"
#include "MemberMap.h"

void DB::ValueCategoryMatcher::debug(int level) const
{
    qCDebug(DBCategoryMatcherLog, "%s%s: %s", qPrintable(spaces(level)), qPrintable(m_category), qPrintable(m_option));
}

DB::ValueCategoryMatcher::ValueCategoryMatcher(const QString &category, const QString &value)
{
    // Unescape doubled "&"s and restore the original value
    QString unEscapedValue = value;
    unEscapedValue.replace(QString::fromUtf8("&&"), QString::fromUtf8("&"));

    m_category = category;
    m_option = unEscapedValue;

    const MemberMap &map = DB::ImageDB::instance()->memberMap();
    const QStringList members = map.members(m_category, m_option, true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    m_members = StringSet(members.begin(), members.end());
#else
    m_members = members.toSet();
#endif
}

bool DB::ValueCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet> &alreadyMatched)
{
    // Only add the tag _option to the alreadyMatched tags,
    // and omit the tags in _members
    if (m_shouldPrepareMatchedSet)
        alreadyMatched[m_category].insert(m_option);

    if (info->hasCategoryInfo(m_category, m_option)) {
        return true;
    }

    if (info->hasCategoryInfo(m_category, m_members))
        return true;
    return false;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
