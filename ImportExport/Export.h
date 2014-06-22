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

#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

#include "ImageManager/ImageClientInterface.h"
#include <KDialog>
#include "Utilities/UniqFilenameMapper.h"
#include <QEventLoop>
#include <QPointer>
#include <DB/FileNameList.h>

class QRadioButton;
class QSpinBox;
class QCheckBox;
class KZip;
class QProgressDialog;

namespace ImportExport
{

enum ImageFileLocation { Inline, ManualCopy, AutoCopy, Link, Symlink };

class Export :public ImageManager::ImageClientInterface {

public:
    static void imageExport(const DB::FileNameList& list);

    Export( const DB::FileNameList& list, const QString& zipFile,
            bool compress, int maxSize,
            ImageFileLocation, const QString& baseUrl,
            bool generateThumbnails,
            bool *ok);
    ~Export();

    static void showUsageDialog();

    // ImageManager::ImageClient callback.
    void pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image) override;

protected:
    void generateThumbnails(const DB::FileNameList& list);
    void copyImages(const DB::FileNameList& list);

private:
    bool* _ok;
    int _filesRemaining;
    int _steps;
    QProgressDialog* _progressDialog;
    KZip* _zip;
    int _maxSize;
    QString _subdir;
    bool _loopEntered;
    ImageFileLocation _location;
    Utilities::UniqFilenameMapper _filenameMapper;
    bool _copyingFiles;
    QString _destdir;
    const QPointer <QEventLoop> _eventLoop;
};

class ExportConfig :public KDialog {
    Q_OBJECT

public:
    ExportConfig();
    QCheckBox* _compress;
    QCheckBox* _generateThumbnails;
    QCheckBox* _enforeMaxSize;
    QSpinBox* _maxSize;

    ImageFileLocation imageFileLocation() const;

private:
    QRadioButton* _include;
    QRadioButton* _manually;
    QRadioButton* _link;
    QRadioButton* _symlink;
    QRadioButton* _auto;
};

}


#endif /* IMPORTEXPORT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
