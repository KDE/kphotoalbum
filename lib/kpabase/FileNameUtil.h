// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef KPABASE_FILENAME_UTIL_H
#define KPABASE_FILENAME_UTIL_H

#include <QString>

namespace DB
{
class FileName;
}
namespace Utilities
{
/**
 * @brief stripEndingForwardSlash removes a trailing '/' from a QString if there is one.
 * @param fileName
 * @return the \p fileName without ending '/'
 * @note The mere usage of this function for actual filename operations seems like a code-smell to me - porting towards robust interfaces as offered by QDir seems prudent.
 */
QString stripEndingForwardSlash(const QString &fileName);

/**
 * @brief fileNameFromUserData creates a FileName from an absolute or relative file name string.
 * It also recognizes local URLs (starting with "file://").
 * @param fileName
 * @return a DB::FileName corresponding to the \p fileName, or a default-constructed FileName if the name is not valid.
 * @attention Requires an initialized SettingsData singleton.
 */
DB::FileName fileNameFromUserData(const QString &fileName);

/**
 * @brief Compute the directory part of a relative or absolute filename.
 * @param fileName a filename
 * @return the \p fileName up to (but excluding) the last occurrence of '/'
 */
QString relativeFolderName(const QString &fileName);
}

#endif /* KPABASE_FILENAME_UTIL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
