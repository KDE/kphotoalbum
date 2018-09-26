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

#ifndef NEWIMAGEFINDER_H
#define NEWIMAGEFINDER_H
#include "ImageInfo.h"
#include "ImageInfoPtr.h"

namespace DB
{
class MD5Map;
class IdList;
class FileNameList;

class NewImageFinder
{
public:
    bool findImages();
    bool calculateMD5sums(const DB::FileNameList& list, DB::MD5Map* map, bool* wasCanceled=nullptr);

protected:
    void searchForNewFiles( const DB::FileNameSet& loadedFiles, QString directory );
    void setupFileVersionDetection();
    void loadExtraFiles();
    void loadExtraFile( const DB::FileName& name, DB::MediaType type );
    void markUnTagged( ImageInfoPtr info );
    bool handleIfImageHasBeenMoved( const DB::FileName& newFileName, const MD5& sum );

private:
    typedef QList< QPair< DB::FileName, DB::MediaType > > LoadList;
    LoadList m_pendingLoad;

    QString m_modifiedFileCompString;
    QRegExp m_modifiedFileComponent;
    QStringList m_originalFileComponents;
};
}

#endif /* NEWIMAGEFINDER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
