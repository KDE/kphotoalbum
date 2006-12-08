/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H
#include <kdialogbase.h>
#include "Settings/SettingsData.h"
#include "DB/MemberMap.h"
class QListView;
class KColorButton;
class QSpinBox;
class KComboBox;
class QLineEdit;
class QListBox;
class KIconButton;
class KPushButton;
class QCheckBox;
class QComboBox;
class QButtonGroup;

namespace KIPI
{
    class ConfigWidget;
}
namespace Exif{
    class TreeView;
}
#ifdef SQLDB_SUPPORT
namespace SQLDB { class SQLSettingsWidget; }
#endif

namespace Settings
{
class ViewerSizeConfig;
class CategoryItem;

class SettingsDialog :public KDialogBase {
    Q_OBJECT

public:
    SettingsDialog( QWidget* parent, const char* name = 0 );
    virtual void show();
    int exec();

public slots:
    void showBackendPage();

signals:
    void changed();

protected slots:
    void slotMyOK();
    void edit( QListBoxItem* );
    void slotLabelChanged( const QString& );
    void slotPreferredViewChanged( int );
    void slotIconChanged( QString );
    void slotNewItem();
    void slotDeleteCurrent();
    void slotCategoryChanged( const QString& );
    void slotGroupSelected( QListBoxItem* );
    void slotAddGroup();
    void slotDelGroup();
    void slotRenameGroup();
    void slotPageChange();
    void thumbnailSizeChanged( int );

protected:
    void createGeneralPage();
    void createThumbNailPage();
    void createOptionGroupsPage();
    void createGroupConfig();
    void enableDisable( bool );
    void saveOldGroup();
    void selectMembers( const QString& );
    void slotCategoryChanged( const QString&, bool saveGroups );
    void setButtonStates();
    void createViewerPage();
    void createPluginPage();
    void createEXIFPage();
    void createDatabaseBackendPage();

private:
    // General page
    KComboBox* _trustTimeStamps;
    QSpinBox* _autosave;
    QCheckBox* _useEXIFRotate;
    QCheckBox* _useEXIFComments;
    QSpinBox* _previewSize;
    QSpinBox* _thumbnailSize;
    QComboBox* _albumCategory;
    QCheckBox* _displayLabels;
    QCheckBox* _displayCategories;
    QCheckBox* _searchForImagesOnStartup;
    QCheckBox* _dontReadRawFilesWithOtherMatchingFile;
    QSpinBox* _thumbnailCache;
    QSpinBox* _autoShowThumbnailView;
    QSpinBox* _barWidth;
    QSpinBox* _barHeight;
    QSpinBox* _backupCount;
    QCheckBox* _compressBackup;
    QCheckBox* _compressedIndexXML;
    QCheckBox* _showSplashScreen;

    // Categories page
    QListBox* _categories;
    QLabel* _labelLabel;
    QLineEdit* _text;
    QLabel* _iconLabel;
    KIconButton* _icon;
    QLabel* _thumbnailSizeInCategoryLabel;
    QSpinBox* _thumbnailSizeInCategory;

    QLabel* _preferredViewLabel;
    QComboBox* _preferredView;
    KPushButton* _delItem;
    CategoryItem* _current;
    QValueList<CategoryItem*> _deleted;

    // Member Groups page
    QComboBox* _category;
    QListBox* _groups;
    QListBox* _members;
    DB::MemberMap _memberMap;
    QString _currentCategory;
    QString _currentGroup;
    QPushButton* _rename;
    QPushButton* _del;

    // Viewer page
    ViewerSizeConfig* _slideShowSetup;
    ViewerSizeConfig* _viewImageSetup;
    QComboBox* _smoothScale;
    QSpinBox* _slideShowInterval;
    QSpinBox* _cacheSize;
    KComboBox* _viewerStandardSize;

#ifdef HASKIPI
    // Plugin config
    KIPI::ConfigWidget* _pluginConfig;
    QCheckBox* _delayLoadingPlugins;
#endif

    // Exif viewer
    Exif::TreeView* _exifForViewer;
    Exif::TreeView* _exifForDialog;

    int _backendPageIndex;
    QButtonGroup* _backendButtons;
#ifdef SQLDB_SUPPORT
    // SQL backend
    SQLDB::SQLSettingsWidget* _sqlSettings;
#endif

};

}

#endif /* OPTIONSDIALOG_H */

