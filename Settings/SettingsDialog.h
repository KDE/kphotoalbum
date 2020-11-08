/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

// KDE includes
#include <KPageDialog>

namespace Exif
{
class TreeView;
}

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
enum class SettingsPage {
    BirthdayPage,
    CategoryPage,
    DatabaseBackendPage,
    ExifPage,
    FileVersionDetectionPage,
    GeneralPage,
    PluginsPage,
    TagGroupsPage,
    ThumbnailsPage,
    ViewerPage
};

class SettingsDialog : public KPageDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent);
    virtual void show();

public slots:
    void activatePage(SettingsPage pageId);

signals:
    void changed();
    void thumbnailSizeChanged();

protected slots:
    void slotMyOK();

private:
    Settings::GeneralPage *m_generalPage;
    Settings::FileVersionDetectionPage *m_fileVersionDetectionPage;
    Settings::ThumbnailsPage *m_thumbnailsPage;
    Settings::CategoryPage *m_categoryPage;
    Settings::TagGroupsPage *m_tagGroupsPage;
    Settings::ViewerPage *m_viewerPage;
    Settings::PluginsPage *m_pluginsPage;
    Settings::ExifPage *m_exifPage;
    Settings::DatabaseBackendPage *m_databaseBackendPage;
    Settings::BirthdayPage *m_birthdayPage;
    QMap<SettingsPage, KPageWidgetItem *> m_pages;

    void keyPressEvent(QKeyEvent *) override;
};
}

#endif /* SETTINGSDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
