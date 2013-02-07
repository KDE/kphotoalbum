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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H
#include <KPageDialog>

namespace KIPI { class ConfigWidget; }
namespace Exif { class TreeView; }

namespace Settings
{
class ViewerSizeConfig;
class CategoryItem;
class CategoryPage;
class SubCategoriesPage;
class GeneralPage;
class ThumbnailsPage;
class ViewerPage;
class FileVersionDetectionPage;
class PluginsPage;
class ExifPage;
class DatabaseBackendPage;

class SettingsDialog :public KPageDialog {
    Q_OBJECT

public:
    SettingsDialog( QWidget* parent );
    virtual void show();

public slots:
    void showBackendPage();

signals:
    void changed();
    void thumbnailSizeChanged();

protected slots:
    void slotMyOK();

private:
    Settings::GeneralPage* _generalPage;
    Settings::FileVersionDetectionPage* _fileVersionDetectionPage;
    Settings::ThumbnailsPage* _thumbnailsPage;
    Settings::CategoryPage* _categoryPage;
    Settings::SubCategoriesPage* _subCategoriesPage;
    Settings::ViewerPage* _viewerPage;
    Settings::PluginsPage* _pluginsPage;
    Settings::ExifPage* _exifPage;
    Settings::DatabaseBackendPage* _databaseBackendPage;
    KPageWidgetItem* _backendPage;
};

}

#endif /* SETTINGSDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
