/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
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
class QSpinBox;
class KComboBox;
class QLineEdit;
class QListBox;
class KIconButton;
class KPushButton;
class OptionGroupItem;

class OptionsDialog :public KDialogBase {
    Q_OBJECT

public:
    OptionsDialog( QWidget* parent, const char* name = 0 );
    virtual void show();

signals:
    void imagePathChanged();
    void changed();

protected slots:
    void slotMyOK();
    void slotBrowseForDirecory();
    void edit( QListBoxItem* );
    void slotLabelChanged( const QString& );
    void slotIconChanged( QString );
    void slotNewItem();
    void slotDeleteCurrent();

protected:
    void createGeneralPage();
    void createOptionGroupsPage();
    void enableDisable( bool );


private:
    // General page
    QSpinBox* _thumbnailSize;
    KComboBox* _trustTimeStamps;
    QLineEdit* _imageDirectory;
    QSpinBox* _autosave;
    QSpinBox* _maxImages;

    // Option Groups page
    QListBox* _optionGroups;
    QLineEdit* _text;
    KIconButton* _icon;
    KPushButton* _delItem;
    OptionGroupItem* _current;
    QValueList<OptionGroupItem*> _deleted;
};


#endif /* OPTIONSDIALOG_H */

