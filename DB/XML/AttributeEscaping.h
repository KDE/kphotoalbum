// SPDX-FileCopyrightText: 2025 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2025 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef ATTRIBUTEESCAPING_H
#define ATTRIBUTEESCAPING_H

#include <QString>

namespace DB
{

/**
 * @brief Unescape a string used as an XML attribute name.
 *
 * @see escape
 *
 * @param str the string to be unescaped
 * @return the unescaped string
 */
QString unescapeAttributeName(const QString &str);

/**
 * @brief Escape problematic characters in a string that forms an XML attribute name.
 *
 * N.B.: Attribute values do not need to be escaped!
 * @see unescape
 *
 * @param str the string to be escaped
 * @return the escaped string
 */
QString escapeAttributeName(const QString &str);
}

#endif // ATTRIBUTEESCAPING_H

// vi:expandtab:tabstop=4 shiftwidth=4:
