/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NEWIMAGEFINDER_H
#define NEWIMAGEFINDER_H
#include "ImageInfo.h"
#include "ImageInfoPtr.h"

namespace DB
{
class MD5Map;
class FileNameList;

class NewImageFinder
{
public:
    bool findImages();
    bool calculateMD5sums(const DB::FileNameList &list, DB::MD5Map *map, bool *wasCanceled = nullptr);

protected:
    void searchForNewFiles(const DB::FileNameSet &loadedFiles, QString directory);
    void setupFileVersionDetection();
    void loadExtraFiles();
    void loadExtraFile(const DB::FileName &name, DB::MediaType type);
    void markUnTagged(ImageInfoPtr info);
    bool handleIfImageHasBeenMoved(const DB::FileName &newFileName, const MD5 &sum);

private:
    typedef QList<QPair<DB::FileName, DB::MediaType>> LoadList;
    LoadList m_pendingLoad;

    QString m_modifiedFileCompString;
    QRegExp m_modifiedFileComponent;
    QStringList m_originalFileComponents;
};
}

#endif /* NEWIMAGEFINDER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
