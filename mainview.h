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

#ifndef MAINVIEW_H
#define MAINVIEW_H
class OptionsDialog;
class ImageConfig;
class QWidgetStack;
class ImageCounter;
class QTimer;
class Browser;
class KTipDialog;
class DeleteDialog;
class QLabel;
class HTMLExportDialog;
class KActionMenu;
class KRadioAction;
namespace KIPI
{
    class PluginLoader;
}

#include "imageinfo.h"
#include <qdict.h>
#include <kmainwindow.h>
#include "thumbnailview.h"
#include "options.h"
#include <kurl.h>

class MainView :public KMainWindow
{
    Q_OBJECT

public:
    MainView( QWidget* parent,  const char* name = 0 );
    static void configureImages( const ImageInfoList& list, bool oneAtATime );
    static MainView* theMainView();
    ImageInfoList selected();
    ImageInfoList currentView();
    ImageSearchInfo currentContext();
    QString currentBrowseCategory() const;

protected slots:
    bool slotExit();
    void slotOptions();
    void slotConfigureAllImages();
    void slotConfigureImagesOneAtATime();
    void slotSave();
    void slotDeleteSelected();
    void slotSearch();
    void slotView( bool reuse = false, bool slideShow = false, bool random = false );
    void slotViewNewWindow();
    void slotSortByDateAndTime();
    void slotChanges();
    void slotLimitToSelected();
    void slotExportToHTML();
    void slotAutoSave();
    void showThumbNails();
    void showBrowser();
    void slotOptionGroupChanged();
    void showTipOfDay();
    void pathChanged( const QString& );
    void runDemo();
    void lockToDefaultScope();
    void setDefaultScopePositive();
    void setDefaultScopeNegative();
    void unlockFromDefaultScope();
    void changePassword();
    void slotConfigureKeyBindings();
    void slotSetFileName( const QString& );
    void slotThumbNailSelectionChanged();
    void reloadThumbNail();
    void slotUpdateViewMenu( Options::ViewSize, Options::ViewType );
    void slotShowNotOnDisk();
    void markDirty();
    void donateMoney();
    void slotRemoveAllThumbnails();
    void slotBuildThumbnails();
    void slotRunSlideShow();
    void slotRunRandomizedSlideShow();
    void slotConfigureToolbars();
    void slotNewToolbarConfig();
    void slotImport();
    void slotExport();
    void delayedInit();
    void slotReenableMessages();
    void slotImagesChanged( const KURL::List& );

protected:
    void configureImages( bool oneAtATime );
    QString welcome();
    virtual void closeEvent( QCloseEvent* e );
    void setupMenuBar();
    void startAutoSaveTimer();
    void createImageConfig();
    void load();
    void save( const QString& fileName );
    virtual void contextMenuEvent( QContextMenuEvent* e );
    void setDirty( bool b );
    void setLocked( bool b );
    void configImages( const ImageInfoList& list, bool oneAtATime );
    void updateStates( bool thumbNailView );
    ImageInfoList getSelectedOnDisk();
    void loadPlugins();

private:
    static MainView* _instance;

    ThumbNailView* _thumbNailView;
    OptionsDialog* _optionsDialog;
    ImageConfig* _imageConfigure;
    bool _dirty;
    bool _autoSaveDirty; // We do not want to continue autosaving the same date
    QWidgetStack* _stack;
    QWidget* _welcome;
    QTimer* _autoSaveTimer;
    Browser* _browser;
    KTipDialog* _tipDialog;
    DeleteDialog* _deleteDialog;
    QLabel* _dirtyIndicator;
    QLabel* _lockedIndicator;
    KAction* _lock;
    KAction* _unlock;
    KAction* _setDefaultPos;
    KAction* _setDefaultNeg;
    HTMLExportDialog* _htmlDialog;
    KAction* _configOneAtATime;
    KAction* _configAllSimultaniously;
    KAction* _view;
    KAction* _sortByDateAndTime;
    KAction* _viewInNewWindow;
    KActionMenu* _viewMenu;
    KRadioAction* _smallListView;
    KRadioAction* _largeListView;
    KRadioAction* _smallIconView;
    KRadioAction* _largeIconView;
    KAction* _generateHtml;
    KAction* _cut;
    KAction* _paste;
    KAction* _deleteSelected;
    KAction* _limitToMarked;
    KAction* _selectAll;
    KAction* _runSlideShow;
    KAction* _runRandomSlideShow;
    KIPI::PluginLoader* _pluginLoader;
};


#endif /* MAINVIEW_H */

