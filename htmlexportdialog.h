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

#ifndef HTMLEXPORTDIALOG_H
#define HTMLEXPORTDIALOG_H
#include <kdialogbase.h>
class KLineEdit;
class QSpinBox;
class QCheckBox;
class QProgressDialog;
class ImageSizeCheckBox;
class QTextEdit;
#include "imageclient.h"
#include <qvaluelist.h>
#include <qcombobox.h>
#include "util.h"

class HTMLExportDialog :public KDialogBase, private ImageClient {
    Q_OBJECT

public:
    HTMLExportDialog( QWidget* parent, const char* name = 0 );
    int exec( const QStringList& list );

protected slots:
    void slotOk();
    void selectDir();
    void slotCancelGenerate();
    void showBrowser();

protected:
    QString createImage( const QString& fileName, int size );
    QString imageName( const QString& fileName, int size );
    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, bool loadedOK );
    bool generate();
    bool generateIndexPage( int width, int height );
    bool generateContextPage( int width, int height, const QString& prevInfo,
                              const QString& info, const QString& nextInfo );
    bool checkVars();
    int calculateSteps();
    QString namePage( int width, int height, const QString& fileName );
    QString nameImage( const QString& fileName, int size );
    bool writeToFile( const QString& fileName, const QString& str );
    bool linkIndexFile();
    QValueList<ImageSizeCheckBox*> activeResolutions();
    void populateThemesCombo();
    void getThemeInfo( QString* baseDir, QString* name, QString* author );
    void createContentPage();
    void createLayoutPage();
    void createDestinationPage();
    QString kimFileName( bool relative );
    QString translateToHTML( const QString& );

private:
    KLineEdit* _title;
    KLineEdit* _baseDir;
    KLineEdit* _baseURL;
    KLineEdit* _destURL;
    KLineEdit* _outputDir;
    QSpinBox* _thumbSize;
    QComboBox *_themeBox;
    QMap<int,QString> _themes;
    QValueList<ImageSizeCheckBox*> _cbs;
    QValueList<ImageSizeCheckBox*> _preferredSizes;
    QTextEdit* _description;
    QSpinBox* _numOfCols;
    QCheckBox* _generateKimFile;

    QStringList _list;
    int _waitCounter;
    int _total;
    QProgressDialog* _progress;
    bool _doneLoading;
    QString _tempDir;
    QMap< QString, QCheckBox* > _whatToIncludeMap;
    Util::UniqNameMap _nameMap;
    int _maxImageSize;
};


#endif /* HTMLEXPORTDIALOG_H */

