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
#include "DB/FileNameList.h"

namespace HTMLGenerator
{
class ImageSizeCheckBox;
class Generator;
class Setup;

class HTMLDialog :public KPageDialog {
    Q_OBJECT

public:
    explicit HTMLDialog( QWidget* parent );
    int exec(const DB::FileNameList& list);

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
    KLineEdit* m_title;
    KLineEdit* m_baseDir;
    KLineEdit* m_baseURL;
    KLineEdit* m_destURL;
    KLineEdit* m_outputDir;
    KLineEdit* m_copyright;
    QCheckBox* m_date;
    QSpinBox* m_thumbSize;
    QTextEdit* m_description;
    QSpinBox* m_numOfCols;
    QCheckBox* m_generateKimFile;
    QCheckBox* m_inlineMovies;
    QCheckBox* m_html5Video;
    QCheckBox* m_html5VideoGenerate;
    QMap<int,QString> m_themes;
    KComboBox* m_themeBox;
    QLabel* m_themeInfo;
    QStringList m_themeAuthors;
    QStringList m_themeDescriptions;
    QMap< QString, QCheckBox* > m_whatToIncludeMap;
    QList<ImageSizeCheckBox*> m_sizeCheckBoxes;
    DB::FileNameList m_list;
};

}

#endif /* HTMLGENERATOR_HTMLDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
