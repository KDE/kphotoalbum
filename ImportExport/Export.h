/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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

#include "ImageManager/ImageClient.h"
#include <KDialog>
#include "Utilities/UniqFilenameMapper.h"
#include <QEventLoop>

namespace DB { class Result; }

class QRadioButton;
class QSpinBox;
class QCheckBox;
class KZip;
class Q3ProgressDialog;

namespace ImportExport
{

enum ImageFileLocation { Inline, ManualCopy, AutoCopy, Link };

class Export :public ImageManager::ImageClient {

public:
    static void imageExport(const DB::Result& list);

    Export( const DB::Result& list, const QString& zipFile,
            bool compress, int maxSize,
            ImageFileLocation, const QString& baseUrl,
            bool generateThumbnails,
            bool *ok);

    static void showUsageDialog();

    // ImageManager::ImageClient callback.
    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, const bool loadedOK);

protected:
    void generateThumbnails(const DB::Result& list);
    void copyImages(const DB::Result& list);

private:
    bool* _ok;
    int _filesRemaining;
    int _steps;
    Q3ProgressDialog* _progressDialog;
    KZip* _zip;
    int _maxSize;
    QString _subdir;
    bool _loopEntered;
    ImageFileLocation _location;
    Utilities::UniqFilenameMapper _filenameMapper;
    bool _copyingFiles;
    QString _destdir;
    QEventLoop _eventLoop;
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
    QRadioButton* _auto;
};

}


#endif /* IMPORTEXPORT_H */

