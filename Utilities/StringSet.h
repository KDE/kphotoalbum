/*
  SPDX-FileCopyrightText: 2007-2010 Tuomas Suutari <thsuut@utu.fi>
  SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef UTILITIES_STRINGSET_H
#define UTILITIES_STRINGSET_H

#include <QSet>
#include <QString>

namespace Utilities
{
/* there is no Set<> anymore since QSet<> is now provided by Qt.
     * For backwards compatibility we still use a useful typedef, the StringSet
     */
typedef QSet<QString> StringSet;
}

#endif /* UTILITIES_STRINGSET_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
