/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BREADCRUMBLIST_H
#define BREADCRUMBLIST_H
#include "Breadcrumb.h"

#include <QList>

namespace Browser
{

/**
 * \brief A List of \ref Breadcrumb's
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 */
class BreadcrumbList : public QList<Breadcrumb>
{
public:
    BreadcrumbList latest() const;
    QString toString() const;
};

}

#endif /* BREADCRUMBLIST_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
