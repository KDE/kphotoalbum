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

#ifndef MAINVIEW_H
#define MAINVIEW_H
class OptionsDialog;
class ImageConfig;
class QWidgetStack;
class ImageCounter;
class QTimer;
class Browser;
class KTipDialog;
#include "imageinfo.h"
#include <qdict.h>
#include <kmainwindow.h>
#include "thumbnailview.h"

class MainView :public KMainWindow
{
    Q_OBJECT

public:
    MainView( QWidget* parent,  const char* name = 0 );

protected slots:
    bool slotExit();
    void slotOptions();
    void slotConfigureAllImages();
    void slotConfigureImagesOneAtATime();
    void slotSave();
    void slotDeleteSelected();
    void slotSearch();
    void slotViewSelected( bool reuse = true );
    void slotViewSelectedNewWindow();
    void slotChanges();
    void slotLimitToSelected();
    void slotExportToHTML();
    void slotDeleteOption( const QString& optionGroup, const QString& which );
    void slotRenameOption( const QString& optionGroup, const QString& oldValue, const QString& newValue );
    void slotAutoSave();
    void showThumbNails();
    void load();
    void showBrowser();
    void slotOptionGroupChanged();
    void showTipOfDay();
    void pathChanged( const QString& );

protected:
    void configureImages( bool oneAtATime );
    ImageInfoList selected();
    void welcome();
    virtual void closeEvent( QCloseEvent* e );
    void setupMenuBar();
    void startAutoSaveTimer();
    void createImageConfig();

private:
    ThumbNailView* _thumbNailView;
    OptionsDialog* _optionsDialog;
    ImageConfig* _imageConfigure;
    bool _dirty;
    QWidgetStack* _stack;
    QWidget* _welcome;
    QTimer* _autoSaveTimer;
    Browser* _browser;
    KTipDialog* _tipDialog;
};


#endif /* MAINVIEW_H */

