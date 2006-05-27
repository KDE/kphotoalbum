/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

#include "ImageManager/ImageClient.h"
#include <kdialogbase.h>
#include "Utilities/Util.h"
class QRadioButton;
class QSpinBox;
class QCheckBox;
class KZip;
class QProgressDialog;

namespace ImportExport
{

enum ImageFileLocation { Inline, ManualCopy, AutoCopy, Link };

class Export :public ImageManager::ImageClient {

public:
    static void imageExport( const QStringList& list);
    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, bool loadedOK );
    Export( const QStringList& list, const QString& zipFile, bool compress, int maxSize,
            ImageFileLocation, const QString& baseUrl, bool& ok, bool generateThumbnails );
    static void showUsageDialog();

protected:
    void generateThumbnails( const QStringList& list );
    void copyImages( const QStringList& list );

private:
    int _filesRemaining;
    int _steps;
    QProgressDialog* _progressDialog;
    bool& _ok;
    KZip* _zip;
    int _maxSize;
    QString _subdir;
    bool _loopEntered;
    ImageFileLocation _location;
    Utilities::UniqNameMap _nameMap;
    bool _copyingFiles;
    QString _destdir;
};

class ExportConfig :public KDialogBase {
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

