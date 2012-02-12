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
#ifndef FILEDETECTION_H
#define FILEDETECTION_H
#include <QWidget>

class KComboBox;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QCheckBox;
namespace Settings
{
class SettingsData;

class FileVersionDetectionPage :public QWidget
{
public:
    FileVersionDetectionPage( QWidget* parent );
    void loadSettings( Settings::SettingsData* );
    void saveSettings( Settings::SettingsData* );

private:
    QCheckBox* _searchForImagesOnStart;
    QCheckBox* _ignoreFileExtension;
    QCheckBox* _skipSymlinks;
    QCheckBox* _skipRawIfOtherMatches;
    QLineEdit* _excludeDirectories; // Directories to exclude
    QCheckBox* _detectModifiedFiles;
    QLineEdit* _modifiedFileComponent;
    QLineEdit* _originalFileComponent;
    QCheckBox* _moveOriginalContents;
    QCheckBox* _autoStackNewFiles;
    QLineEdit* _copyFileComponent;
    QLineEdit* _copyFileReplacementComponent;
};


}


#endif /* FILEDETECTION_H */

