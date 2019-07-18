/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CATEGORYMATCHER_H
#define CATEGORYMATCHER_H

#include <DB/ImageInfoPtr.h>
#include <Utilities/StringSet.h>

#include <QMap>

namespace DB
{
class ImageInfo;

using Utilities::StringSet;

/**
   \brief Base class for components of the image searching frame work.

   The matcher component must implement \ref eval which tells if the given
   image is matched by this component.

   If the over all search contains a "No other" part (as in Jesper and no
   other people", then we need to collect items of the category items seen. This,
   however, is rather expensive, so this collection is only turned on in
   that case.

*/
class CategoryMatcher
{
public:
    CategoryMatcher();
    virtual ~CategoryMatcher() {}
    virtual void debug(int level) const = 0;

    virtual bool eval(ImageInfoPtr, QMap<QString, StringSet> &alreadyMatched) = 0;
    virtual void setShouldCreateMatchedSet(bool);

protected:
    QString spaces(int level) const;

    bool m_shouldPrepareMatchedSet;
};

}

#endif /* CATEGORYMATCHER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
