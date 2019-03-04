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

// KDE includes
#include <KPageDialog>

namespace KIPI { class ConfigWidget; }
namespace Exif { class TreeView; }

namespace Settings
{
class ViewerSizeConfig;
class CategoryItem;
class CategoryPage;
class TagGroupsPage;
class GeneralPage;
class ThumbnailsPage;
class ViewerPage;
class FileVersionDetectionPage;
class PluginsPage;
class ExifPage;
class DatabaseBackendPage;
class BirthdayPage;

/**
 * @brief The SettingsPage enum has a value for every settings sub-page.
 * It is used for SettingsDialog::setPage().
 */
enum class SettingsPage
{
    BirthdayPage
    , CategoryPage
    , DatabaseBackendPage
    , ExifPage
    , FileVersionDetectionPage
    , GeneralPage
    , PluginsPage
    , TagGroupsPage
    , ThumbnailsPage
    , ViewerPage
};

class SettingsDialog :public KPageDialog {
    Q_OBJECT

public:
    explicit SettingsDialog( QWidget* parent );
    virtual void show();

public slots:
    void activatePage(SettingsPage pageId);

signals:
    void changed();
    void thumbnailSizeChanged();

protected slots:
    void slotMyOK();

private:
    Settings::GeneralPage* m_generalPage;
    Settings::FileVersionDetectionPage* m_fileVersionDetectionPage;
    Settings::ThumbnailsPage* m_thumbnailsPage;
    Settings::CategoryPage* m_categoryPage;
    Settings::TagGroupsPage* m_tagGroupsPage;
    Settings::ViewerPage* m_viewerPage;
    Settings::PluginsPage* m_pluginsPage;
    Settings::ExifPage* m_exifPage;
    Settings::DatabaseBackendPage* m_databaseBackendPage;
    Settings::BirthdayPage* m_birthdayPage;
    QMap<SettingsPage, KPageWidgetItem*> m_pages;

    void keyPressEvent(QKeyEvent*) override;
};

}

#endif /* SETTINGSDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
