/* SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef UTIL_H
#define UTIL_H

#include <QString>

/**
 * \file Utility functions for copying and linking files.
 *
 * \see FileNameUtils.h
 */

namespace Utilities
{
bool copyOrOverwrite(const QString &from, const QString &to);
bool makeSymbolicLink(const QString &from, const QString &to);
bool makeHardLink(const QString &from, const QString &to);
}

#endif /* UTIL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
