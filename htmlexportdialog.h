/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
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

#ifndef HTMLEXPORTDIALOG_H
#define HTMLEXPORTDIALOG_H
#include <kdialogbase.h>
class KLineEdit;
class QSpinBox;
class QCheckBox;
class QProgressDialog;
class QSlider;
class MyCheckBox;
#include "imageinfo.h"
#include "imageclient.h"
#include <qvaluelist.h>

class HTMLExportDialog :public KDialogBase, private ImageClient {
    Q_OBJECT

public:
    HTMLExportDialog( const ImageInfoList& list, QWidget* parent, const char* name = 0 );

protected slots:
    void slotOk();
    void selectDir();
    void slotCancelGenerate();

protected:
    QString createImage( ImageInfo* info, int size );
    QString imageName( const QString& fileName, int size );
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QImage& );
    bool generate();
    QString outputDir( bool showErr );

private:
    KLineEdit* _title;
    KLineEdit* _baseDir;
    KLineEdit* _baseURL;
    KLineEdit* _outputDir;
    QSpinBox* _thumbSize;
    QCheckBox* _generateToolTips;
    QSlider* _numOfCols;
    QValueList<MyCheckBox*> _cbs;

    ImageInfoList _list;
    int _waitCounter;
    int _total;
    QProgressDialog* _progress;
    bool _doneLoading;
};


#endif /* HTMLEXPORTDIALOG_H */

