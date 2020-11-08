/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGEDATECOLLECTION_H
#define IMAGEDATECOLLECTION_H
#include "ImageDate.h"

#include <QExplicitlySharedDataPointer>
#include <qmap.h>
namespace DB
{

class ImageCount
{
public:
    ImageCount(int exact, int rangeMatch)
        : mp_exact(exact)
        , mp_rangeMatch(rangeMatch)
    {
    }
    ImageCount() {}

    int mp_exact;
    int mp_rangeMatch;
};

class ImageDateCollection : public QSharedData
{
public:
    virtual ~ImageDateCollection() {}
    virtual ImageCount count(const ImageDate &range) = 0;
    virtual Utilities::FastDateTime lowerLimit() const = 0;
    virtual Utilities::FastDateTime upperLimit() const = 0;
};

}

#endif /* IMAGEDATECOLLECTION_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
