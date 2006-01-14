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

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H
#include <kdialogbase.h>
#include "options.h"
#include "membermap.h"
#include <config.h>
class QListView;
class KColorButton;
class QSpinBox;
class KComboBox;
class QLineEdit;
class QListBox;
class KIconButton;
class KPushButton;
class OptionGroupItem;
class QCheckBox;
class QComboBox;
class ViewerSizeConfig;
namespace KIPI
{
    class ConfigWidget;
}
namespace Exif{
    class TreeView;
}
class OptionsDialog :public KDialogBase {
    Q_OBJECT

public:
    OptionsDialog( QWidget* parent, const char* name = 0 );
    virtual void show();
    int exec();

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

private:
    // General page
    KComboBox* _trustTimeStamps;
    QSpinBox* _autosave;
    QCheckBox* _useEXIFRotate;
    QCheckBox* _useEXIFComments;
    QSpinBox* _previewSize;
    QComboBox* _albumCategory;
    QCheckBox* _displayLabels;
    QCheckBox* _searchForImagesOnStartup;
    QSpinBox* _thumbnailCache;
    QSpinBox* _autoShowThumbnailView;
    QSpinBox* _barWidth;
    QSpinBox* _barHeight;
    QSpinBox* _backupCount;
    QCheckBox* _compressBackup;
    QCheckBox* _compressedIndexXML;

    // Categories page
    QListBox* _categories;
    QLineEdit* _text;
    KIconButton* _icon;
    QLabel* _preferredViewLabel;
    QComboBox* _preferredView;
    KPushButton* _delItem;
    OptionGroupItem* _current;
    QValueList<OptionGroupItem*> _deleted;

    // Member Groups page
    QComboBox* _category;
    QListBox* _groups;
    QListBox* _members;
    MemberMap _memberMap;
    QString _currentCategory;
    QString _currentGroup;
    QPushButton* _rename;
    QPushButton* _del;

    // Viewer page
    ViewerSizeConfig* _slideShowSetup;
    ViewerSizeConfig* _viewImageSetup;
    QSpinBox* _slideShowInterval;
    QSpinBox* _cacheSize;

#ifdef HASKIPI
    // Plugin config
    KIPI::ConfigWidget* _pluginConfig;
    QCheckBox* _delayLoadingPlugins;
#endif

    // Exif viewer
    Exif::TreeView* _exifForViewer;
    Exif::TreeView* _exifForDialog;
};


#endif /* OPTIONSDIALOG_H */

