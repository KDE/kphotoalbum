/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef CATEGORYPTR_H
#define CATEGORYPTR_H
#include <QExplicitlySharedDataPointer>

namespace DB
{
class Category;
typedef QExplicitlySharedDataPointer<Category> CategoryPtr;
}

#endif /* CATEGORYPTR_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
