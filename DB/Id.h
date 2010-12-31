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
#ifndef DB_RESULTID_H
#define DB_RESULTID_H

#include "RawId.h"
#include "IdList.h"
#include "ImageInfoPtr.h"

namespace DB
{
class Id {
public:
    static const Id null;

    /* The default constructor creates a 'null' instance of Id.
     * Consider using DB::Id::null instance instead
     */
    Id();

    /** Construct with given rawId and context.
     *
     * \pre !context.isEmpty()
     */
    Id(RawId rawId, const IdList& context);

    static Id createContextless(RawId rawId)
    {
        return Id(rawId);
    }

    RawId rawId() const;
    bool isNull() const;

    /** Get context of this.
     *
     * \note returned context might be empty
     */
    const IdList& context() const;

    inline bool operator==(const Id& other) const {
        return other._rawId == _rawId;  // we're only interested in the id.
    }
    inline bool operator!=(const Id& other) const {
        return other._rawId != _rawId;  // we're only interested in the id.
    }
    inline bool operator<(const Id& other) const {
        return _rawId < other._rawId;
    }

    /**
     * Convenience method: fetch the associated ImageInfo for this ID or
     * a ImageInfoPtr(NULL) if this id is Id::null
     */
    ImageInfoPtr fetchInfo() const;

 private:
    explicit Id(RawId rawId)
        : _rawId(rawId)
        , _context()
    {
        Q_ASSERT(!isNull());
    }

    RawId _rawId;
    IdList _context;
};
}  // namespace DB

/*
 * qHash() so that we can put the Id in a QSet<> or QHash<> as key.
 *
 * The Qt documentation talks about putting this in the global namespace. But
 * that does not work.
 *
 * Due to some name lookup wierdness, it only works if we enforce this method
 * to be declared before including <QHash> (hard to do, because QHash might be
 * implicitly included in any header before, i.e. that means that Id.h
 * would need to be included always as first header), or just put it in the
 * DB namespace. Either this is a problem in gcc or in Qt (forgot to explicitly
 * address the global namesapce with ::qHash() ?).
 * TODO(hzeller): figure out a better solution.
 * ( http://gcc.gnu.org/bugzilla/show_bug.cgi?id=26311 )
 */
namespace DB {
inline unsigned int qHash(const DB::Id& id)
{
    return ::qHash(id.rawId());
}
}  // namespace DB

#endif /* DB_RESULTID_H */

