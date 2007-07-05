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
#include <KPageDialog>
#include "Settings/SettingsData.h"
#include "DB/MemberMap.h"
//Added by qt3to4:
#include <Q3ValueList>
#include <QLabel>
#include <Q3ListBoxItem>
class Q3ListView;
class KColorButton;
class QSpinBox;
class KComboBox;
class QLineEdit;
class Q3ListBox;
class KIconButton;
class KPushButton;
class QCheckBox;
class QComboBox;
class Q3ButtonGroup;

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

#ifdef TEMPORARILY_REMOVED
class SettingsDialog :public KDialog {
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
    void edit( Q3ListBoxItem* );
    void slotLabelChanged( const QString& );
    void slotPreferredViewChanged( int );
    void slotIconChanged( QString );
    void slotNewItem();
    void slotDeleteCurrent();
    void slotCategoryChanged( const QString& );
    void slotGroupSelected( Q3ListBoxItem* );
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
    QComboBox* _albumCategory;
    QCheckBox* _searchForImagesOnStartup;
    QCheckBox* _dontReadRawFilesWithOtherMatchingFile;
    QSpinBox* _barWidth;
    QSpinBox* _barHeight;
    QSpinBox* _backupCount;
    QCheckBox* _compressBackup;
    QCheckBox* _compressedIndexXML;
    QCheckBox* _showSplashScreen;

    // Thumbnail Page
    QSpinBox* _previewSize;
    QSpinBox* _thumbnailSize;
    QCheckBox* _thumbnailDarkBackground;
    QCheckBox* _displayLabels;
    QCheckBox* _displayCategories;
    QCheckBox* _thumbnailDisplayGrid;
    QSpinBox* _autoShowThumbnailView;
    QSpinBox* _thumbnailCache;
    KComboBox* _thumbnailAspectRatio;
    QSpinBox* _thumbnailSpace;

    // Categories page
    Q3ListBox* _categories;
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
    Q3ValueList<CategoryItem*> _deleted;

    // Member Groups page
    QComboBox* _category;
    Q3ListBox* _groups;
    Q3ListBox* _members;
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
    Q3ButtonGroup* _backendButtons;
#ifdef SQLDB_SUPPORT
    // SQL backend
    SQLDB::SQLSettingsWidget* _sqlSettings;
#endif

};
#endif

}

#endif /* OPTIONSDIALOG_H */

