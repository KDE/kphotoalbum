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

#ifndef IMAGECONFIG_H
#define IMAGECONFIG_H
#include "imageinfo.h"
#include "imageclient.h"
#include "listselect.h"
#include "imagesearchinfo.h"
#include <kdockwidget.h>
#include <qspinbox.h>
#include "imagepreview.h"
#include "editor.h"

class QSplitter;
class Viewer;
class QPushButton;
class KLineEdit;
class KDockWidget;

class ImageConfig :public KDockMainWindow {
    Q_OBJECT
public:
    ImageConfig( QWidget* parent, const char* name = 0 );
    void configure( ImageInfoList list,  bool oneAtATime );
    ImageSearchInfo search();

signals:
    void changed();
    void deleteOption( const QString& optionGroup, const QString& which );
    void renameOption( const QString& optionGroup, const QString& oldValue, const QString& newValue );

protected slots:
    void displayImage();
    void slotRevert();
    void slotPrev();
    void slotNext();
    void slotOK();
    void slotCancel();
    void slotClear();
    void viewerDestroyed();
    void slotOptions();
    void slotSaveWindowSetup();

protected:
    enum SetupType { SINGLE, MULTIPLE, SEARCH };
    void load();
    void save();
    void setup();
    void loadInfo( const ImageSearchInfo& );
    int exec();
    virtual void closeEvent( QCloseEvent* );
    void showTornOfWindows();
    void hideTornOfWindows();
    virtual bool eventFilter( QObject*, QEvent* );
    KDockWidget* createListSel( const QString& optionGroup );

private:
    ImageInfoList _origList;
    QValueList<ImageInfo> _editList;
    int _current;
    SetupType _setup;
    QPtrList< ListSelect > _optionList;
    ImageSearchInfo _oldSearch;
    QSplitter* _splitter;
    Viewer* _viewer;
    int _accept;
    QValueList<KDockWidget*> _dockWidgets;
    QValueList<KDockWidget*> _tornOfWindows;

    // Widgets
    KLineEdit* _imageLabel;
    QSpinBox* _dayStart;
    QSpinBox* _dayEnd;
    QSpinBox* _yearStart;
    QSpinBox* _yearEnd;
    QComboBox* _monthStart;
    QComboBox* _monthEnd;
    ImagePreview* _preview;
    QPushButton* _revertBut;
    QPushButton* _okBut;
    QPushButton* _prevBut;
    QPushButton* _nextBut;
    Editor* _description;

};

#endif /* IMAGECONFIG_H */

