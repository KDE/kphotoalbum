/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H
#include <kdialogbase.h>
#include "options.h"
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
    void createOptionGroupsPage();
    void createGroupConfig();
    void enableDisable( bool );
    void saveOldGroup();
    void selectMembers( const QString& );
    void slotCategoryChanged( const QString&, bool saveGroups );
    void setButtonStates();
    void createViewerPage();

private:
    // General page
    QSpinBox* _thumbnailSize;
    KComboBox* _trustTimeStamps;
    QSpinBox* _autosave;
    QSpinBox* _maxImages;
    QCheckBox* _useEXIFRotate;
    QCheckBox* _useEXIFComments;
    QSpinBox* _previewSize;
    QComboBox* _albumCategory;

    // Option Groups page
    QListBox* _optionGroups;
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
};


#endif /* OPTIONSDIALOG_H */

