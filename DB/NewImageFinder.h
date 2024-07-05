// SPDX-FileCopyrightText: 2003-2012 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007-2008 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2007-2010 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2008 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2011 Andreas Neustifter <andreas.neustifter@gmail.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2017-2018 Robert Krawitz <rlk@alum.mit.edu>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef NEWIMAGEFINDER_H
#define NEWIMAGEFINDER_H
#include "ImageInfo.h"
#include "ImageInfoPtr.h"

#include <QMutex>
#include <QRegularExpression>

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
    QRegularExpression m_modifiedFileComponent;
    QStringList m_originalFileComponents;
    static QMutex s_imageFinderLock; ///< Only one NewImageFinder can accesss the database at any time!
};
}

#endif /* NEWIMAGEFINDER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
