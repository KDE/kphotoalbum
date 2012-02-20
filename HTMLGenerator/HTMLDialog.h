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

#ifndef HTMLGENERATOR_HTMLDIALOG_H
#define HTMLGENERATOR_HTMLDIALOG_H
#include <KPageDialog>
#include <KComboBox>
class QLabel;
class QComboBox;
class QTextEdit;
class KLineEdit;
class QSpinBox;
class QCheckBox;
#include "DB/IdList.h"

namespace HTMLGenerator
{
class ImageSizeCheckBox;
class Generator;
class Setup;

class HTMLDialog :public KPageDialog {
    Q_OBJECT

public:
    HTMLDialog( QWidget* parent );
    int exec(const DB::IdList& list);

protected slots:
    void slotOk();
    void selectDir();
    void displayThemeDescription(int);

protected:
    bool checkVars();
    Setup setup() const;
    QList<ImageSizeCheckBox*> activeResolutions() const;
    QString activeSizes() const;
    QString includeSelections() const;
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
    KLineEdit* _copyright;
    QCheckBox* _date;
    QSpinBox* _thumbSize;
    QTextEdit* _description;
    QSpinBox* _numOfCols;
    QCheckBox* _generateKimFile;
    QCheckBox* _inlineMovies;
    QMap<int,QString> _themes;
    KComboBox* _themeBox;
    QLabel* _themeInfo;
    QStringList _themeAuthors;
    QStringList _themeDescriptions;
    QMap< QString, QCheckBox* > _whatToIncludeMap;
    QList<ImageSizeCheckBox*> _cbs;
    DB::IdList _list;
};

}

#endif /* HTMLGENERATOR_HTMLDIALOG_H */

