// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef KPABASE_FILENAME_UTIL_H
#define KPABASE_FILENAME_UTIL_H

#include <QString>

namespace Utilities
{
/**
 * @brief stripEndingForwardSlash removes a trailing '/' from a QString if there is one.
 * @param fileName
 * @return the \p fileName without ending '/'
 * @note The mere usage of this function for actual filename operations seems like a code-smell to me - porting towards robust interfaces as offered by QDir seems prudent.
 */
QString stripEndingForwardSlash(const QString &fileName);

// FIXME(jzarl): this is only used internally by imageFileNameToAbsolute - remove once that one is no longer needed.
QString absoluteImageFileName(const QString &relativeName);
// FIXME(jzarl): only used in MainWindow::Window::slotShowListOfFiles - please replace
QString imageFileNameToAbsolute(const QString &fileName);

/**
 * @brief Compute the directory part of a relative or absolute filename.
 * @param fileName a filename
 * @return the \p fileName up to (but excluding) the last occurrence of '/'
 */
QString relativeFolderName(const QString &fileName);
}

#endif /* KPABASE_FILENAME_UTIL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
