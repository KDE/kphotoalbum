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
#include <QSet>
#include "ImageInfo.h"
#include "ImageInfoPtr.h"

namespace DB
{
class MD5Map;
class IdList;


class NewImageFinder
{
public:
    bool findImages();
    bool calculateMD5sums(const DB::IdList& list, DB::MD5Map* map, bool* wasCanceled=0);

protected:
    void searchForNewFiles( const QSet<QString>& loadedFiles, QString directory );
    void setupFileVersionDetection();
    void loadExtraFiles();
    ImageInfoPtr loadExtraFile( const QString& name, DB::MediaType type );
    void markUnTagged( ImageInfoPtr info );

private:
    typedef QList< QPair< QString, DB::MediaType > > LoadList;
    LoadList _pendingLoad;

    QString _modifiedFileCompString;
    QRegExp _modifiedFileComponent;
    QStringList _originalFileComponents;
};
}

#endif /* NEWIMAGEFINDER_H */

