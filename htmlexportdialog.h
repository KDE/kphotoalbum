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

#ifndef HTMLEXPORTDIALOG_H
#define HTMLEXPORTDIALOG_H
#include <kdialogbase.h>
class KLineEdit;
class QSpinBox;
class QCheckBox;
class QProgressDialog;
class QSlider;
class MyCheckBox;
class QTextEdit;
#include "imageinfo.h"
#include "imageclient.h"
#include <qvaluelist.h>
#include <qcombobox.h>

class HTMLExportDialog :public KDialogBase, private ImageClient {
    Q_OBJECT

public:
    HTMLExportDialog( QWidget* parent, const char* name = 0 );
    int exec( const ImageInfoList& list );

protected slots:
    void slotOk();
    void selectDir();
    void slotCancelGenerate();
    void showBrowser();

protected:
    QString createImage( ImageInfo* info, int size );
    QString imageName( const QString& fileName, int size );
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QImage& );
    bool generate();
    bool generateIndexPage( int width, int height );
    bool generateContextPage( int width, int height, ImageInfo* prevInfo,
                              ImageInfo* info, ImageInfo* nextInfo );
    bool checkVars();
    int calculateSteps();
    QString namePage( int width, int height, const QString& fileName );
    QString nameThumbNail( ImageInfo* info, int size );
    bool writeToFile( const QString& fileName, const QString& str );
    bool linkIndexFile();
    QValueList<MyCheckBox*> activeResolutions();
    void populateThemesCombo();
    void getThemeInfo( QString* baseDir, QString* name, QString* author );

private:
    KLineEdit* _title;
    KLineEdit* _baseDir;
    KLineEdit* _baseURL;
    KLineEdit* _outputDir;
    QSpinBox* _thumbSize;
    QSlider* _numOfCols;
    QComboBox *_themeBox;
    QMap<int,QString> _themes;
    QValueList<MyCheckBox*> _cbs;
    QValueList<MyCheckBox*> _preferredSizes;
    QTextEdit* _description;

    ImageInfoList _list;
    int _waitCounter;
    int _total;
    QProgressDialog* _progress;
    bool _doneLoading;
    QString _tempDir;
};


#endif /* HTMLEXPORTDIALOG_H */

