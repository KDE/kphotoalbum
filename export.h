/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

#include <imageinfo.h>
#include "imageclient.h"
#include <kdialogbase.h>
class QSpinBox;
class QCheckBox;
class KZip;
class QProgressDialog;

class Export :public ImageClient {

public:
    static void imageExport( const ImageInfoList& list);
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QImage& );

protected:
    QCString createIndexXML( const ImageInfoList& );
    void generateThumbnails( const ImageInfoList& list );
    void copyImages( const ImageInfoList& list );

private:
    Export(  const ImageInfoList& list, bool compress, int maxSize );
    int _filesRemaining;
    int _steps;
    QProgressDialog* _progressDialog;
    bool _ok;
    KZip* _zip;
    QMap<QString,QString> _map;
    int _maxSize;
    QString _subdir;
    bool _loopEntered;
};

class ExportConfig :public KDialogBase {
    Q_OBJECT

public:
    ExportConfig();
    QCheckBox* _compress;
    QCheckBox* _enforeMaxSize;
    QSpinBox* _maxSize;

protected slots:
    void slotHelp();
};


#endif /* IMPORTEXPORT_H */

