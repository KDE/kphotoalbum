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

#ifndef IMAGECONFIG_H
#define IMAGECONFIG_H
#include "imageclient.h"
#include "listselect.h"
#include "imagesearchinfo.h"
#include <kdockwidget.h>
#include <qspinbox.h>
#include "editor.h"
#include <qdialog.h>
#include <ktimewidget.h>
#include "imageinfolist.h"

class ImageInfo;
class QSplitter;
class Viewer;
class QPushButton;
class KLineEdit;
class KDockWidget;
class KDatePicker;
class KDateEdit;
class ImagePreview;
class KPushButton;

class ImageConfig :public QDialog {
    Q_OBJECT
public:
    ImageConfig( QWidget* parent, const char* name = 0 );
    int configure( ImageInfoList list,  bool oneAtATime );
    ImageSearchInfo search( ImageSearchInfo* search = 0 );
    bool thumbnailShouldReload() const;

signals:
    void changed();
    void deleteMe();

protected slots:
    void slotRevert();
    void slotPrev();
    void slotNext();
    void slotOK();
    void slotClear();
    void viewerDestroyed();
    void slotOptions();
    void slotSaveWindowSetup();
    void slotDeleteOption( Category*, const QString& );
    void slotRenameOption( Category* , const QString& , const QString&  );
    virtual void reject();
    void rotateLeft();
    void rotateRight();
    void rotate( int angle );
    void slotAddTimeInfo();
    void slotDeleteImage();
    void slotRecetLayout();
    void slotStartDateChanged( const ImageDate& );

protected:
    enum SetupType { SINGLE, MULTIPLE, SEARCH };
    void load();
    void writeToInfo();
    void setup();
    void loadInfo( const ImageSearchInfo& );
    int exec();
    virtual void closeEvent( QCloseEvent* );
    void showTornOfWindows();
    void hideTornOfWindows();
    virtual bool eventFilter( QObject*, QEvent* );
    KDockWidget* createListSel( const QString& category );
    bool hasChanges();
    void showHelpDialog( SetupType );
    virtual void resizeEvent( QResizeEvent* );
    virtual void moveEvent ( QMoveEvent * );
    void setupFocus();

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
    bool _thumbnailShouldReload;

    // Widgets
    KDockMainWindow* _dockWindow;
    KLineEdit* _imageLabel;
    KDateEdit* _startDate;
    KDateEdit* _endDate;

    ImagePreview* _preview;
    QPushButton* _revertBut;
    KPushButton* _okBut;
    QPushButton* _prevBut;
    QPushButton* _nextBut;
    QPushButton* _rotateLeft;
    QPushButton* _rotateRight;
    QPushButton* _delBut;
    Editor* _description;
    KTimeWidget* _time;
    QPushButton* _addTime;
};

#endif /* IMAGECONFIG_H */

