/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MD5MAP_H
#define MD5MAP_H
#include "FileName.h"
#include "MD5.h"

#include <qhash.h>
#include <qstring.h>

namespace DB
{
typedef QHash<MD5, DB::FileName> MD5FileMap;
typedef QHash<DB::FileName, MD5> FileMD5Map;

/**
   This class may be overridden by a which wants to store md5 information
   directly in a database, rather than in a map in memory.
**/
class MD5Map
{
public:
    virtual ~MD5Map() {}
    virtual void insert(const MD5 &md5sum, const DB::FileName &fileName);
    virtual DB::FileName lookup(const MD5 &md5sum) const;
    virtual MD5 lookupFile(const DB::FileName &fileName) const;
    virtual bool contains(const MD5 &md5sum) const;
    virtual bool containsFile(const DB::FileName &fileName) const;
    virtual void clear();
    virtual DB::FileNameSet diff(const MD5Map &other) const;

private:
    MD5FileMap m_map;
    FileMD5Map m_i_map;
};

}

#endif /* MD5MAP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
