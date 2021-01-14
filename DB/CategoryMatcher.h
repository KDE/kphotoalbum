/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CATEGORYMATCHER_H
#define CATEGORYMATCHER_H

#include "ImageInfoPtr.h"

#include <kpabase/StringSet.h>

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
    virtual ~CategoryMatcher() { }
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
