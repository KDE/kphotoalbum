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

#ifndef HTMLGENERATOR_HTMLDIALOG_H
#define HTMLGENERATOR_HTMLDIALOG_H
#include <kdialogbase.h>
class KLineEdit;
class QSpinBox;
class QCheckBox;
class QProgressDialog;
class QTextEdit;
#include <qvaluelist.h>
#include <qcombobox.h>
#include "Utilities/Util.h"

namespace HTMLGenerator
{
class ImageSizeCheckBox;
class Generator;
class Setup;

class HTMLDialog :public KDialogBase {
    Q_OBJECT

public:
    HTMLDialog( QWidget* parent, const char* name = 0 );
    int exec( const QStringList& list );

protected slots:
    void slotOk();
    void selectDir();

protected:
    bool checkVars();
    Setup setup() const;
    QValueList<ImageSizeCheckBox*> activeResolutions() const;
    void populateThemesCombo();
    void createContentPage();
    void createLayoutPage();
    void createDestinationPage();

private:
    KLineEdit* _title;
    KLineEdit* _baseDir;
    KLineEdit* _baseURL;
    KLineEdit* _destURL;
    KLineEdit* _outputDir;
    QSpinBox* _thumbSize;
    QTextEdit* _description;
    QSpinBox* _numOfCols;
    QCheckBox* _generateKimFile;
    QCheckBox* _inlineMovies;
    QMap<int,QString> _themes;
    QComboBox *_themeBox;
    QMap< QString, QCheckBox* > _whatToIncludeMap;
    QValueList<ImageSizeCheckBox*> _cbs;
    QStringList _list;
};

}

#endif /* HTMLGENERATOR_HTMLDIALOG_H */

