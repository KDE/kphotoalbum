#include <Browser/BreadcrumbList.h>
#include <QContextMenuEvent>
#include <QMoveEvent>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QLabel>
#include <KAction>
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef MAINVIEW_H
#define MAINVIEW_H
class BreadcrumbViewer;
class KToggleAction;
class Q3WidgetStack;
class QTimer;
class KTipDialog;
class QLabel;
class KActionMenu;

#include "DB/ImageInfoList.h"
#include <kmainwindow.h>
#include "Settings/SettingsData.h"
#include <kurl.h>
#include <kxmlguiwindow.h>
#include <QPointer>
#include <config-kpa-kipi.h>
#ifdef HASKIPI
#  include <libkipi/pluginloader.h>
#endif
#include "DB/Result.h"
#include "DB/ResultId.h"

namespace Plugins { class Interface; }
namespace ThumbnailView { class ThumbnailFacade; }
namespace Browser{ class BrowserWidget; }
namespace AnnotationDialog { class Dialog; }
namespace Settings { class SettingsDialog; }
namespace DateBar { class DateBarWidget; }
namespace HTMLGenerator { class HTMLDialog; }

namespace MainWindow
{
class DeleteDialog;
class TokenEditor;
class ImageCounter;
class DirtyIndicator;

class Window :public KXmlGuiWindow
{
    Q_OBJECT

public:
    Window( QWidget* parent );
    ~Window();
    static void configureImages( const DB::ImageInfoList& list, bool oneAtATime );
    static Window* theMainWindow();
    DB::Result selected(bool keepSortOrderOfDatabase=false);
    DB::ImageSearchInfo currentContext();
    QString currentBrowseCategory() const;
    void setStackHead( const DB::ResultId image );

public slots:
    void showThumbNails(const DB::Result& items);
    void loadPlugins();

protected slots:
    void showThumbNails();
    bool slotExit();
    void slotOptions();
    void slotConfigureAllImages();
    void slotConfigureImagesOneAtATime();
    void slotCreateImageStack();
    void slotUnStackImages();
    void slotSetStackHead();
    void slotSave();
    void slotCopySelectedURLs();
    void slotDeleteSelected();
    void slotReReadExifInfo();
    void slotSearch();
    void slotView( bool reuse = true, bool slideShow = false, bool random = false );
    void slotViewNewWindow();
    void slotSortByDateAndTime();
    void slotLimitToSelected();
    void slotExportToHTML();
    void slotAutoSave();
    void showBrowser();
    void slotOptionGroupChanged();
    void showTipOfDay();
    void runDemo();
    void lockToDefaultScope();
    void setDefaultScopePositive();
    void setDefaultScopeNegative();
    void unlockFromDefaultScope();
    void changePassword();
    void slotConfigureKeyBindings();
    void slotSetFileName( const DB::ResultId& );
    void slotThumbNailSelectionChanged(int selectionSize);
    void reloadThumbnails(bool flushCache);
    void reloadThumbnailsAndFlushCache();
    void reloadThumbnailsAfterRotation();
    void slotUpdateViewMenu( DB::Category::ViewType );
    void slotShowNotOnDisk();
    void slotBuildThumbnails();
    void slotRunSlideShow();
    void slotRunRandomizedSlideShow();
    void slotConfigureToolbars();
    void slotNewToolbarConfig();
    void slotImport();
    void slotExport();
    void delayedInit();
    void slotReenableMessages();
    void slotImagesChanged( const KUrl::List& );
    void slotSelectionChanged(int count);
    void plug();
    void slotRemoveTokens();
    void slotShowListOfFiles();
    void updateDateBar( const Browser::BreadcrumbList& );
    void updateDateBar();
    void slotShowImagesWithInvalidDate();
    void slotShowImagesWithChangedMD5Sum();
    void showDateBarTip( const QString& );
    void slotJumpToContext();
    void setDateRange( const DB::ImageDate& );
    void clearDateRange();
    void startAutoSaveTimer();
    void convertBackend();
    void slotRecalcCheckSums();
    void slotShowExifInfo();
    void showFeatures();
    void showImage( const DB::ResultId& fileName );
    void slotOrderIncr();
    void slotOrderDecr();
    void slotRotateSelectedLeft();
    void slotRotateSelectedRight();
    void rotateSelected( int angle );
    void showVideos();
    void slotStatistics();
    void slotRecreateExifDB();

protected:
    void configureImages( bool oneAtATime );
    QString welcome();
    virtual void closeEvent( QCloseEvent* e );
    virtual void resizeEvent( QResizeEvent* );
    virtual void moveEvent ( QMoveEvent * );
    void setupMenuBar();
    void createAnnotationDialog();
    bool load();
    virtual void contextMenuEvent( QContextMenuEvent* e );
    void setLocked( bool b, bool force );
    void configImages( const DB::ImageInfoList& list, bool oneAtATime );
    void updateStates( bool thumbNailView );
    DB::Result selectedOnDisk();
    void setupPluginMenu();
    void launchViewer(const DB::Result& mediaList, bool reuse, bool slideShow, bool random);
    void setupStatusBar();
    void setPluginMenuState( const char* name, const QList<QAction*>& actions );
    void createSarchBar();

private:
    static Window* _instance;

    ThumbnailView::ThumbnailFacade* _thumbnailView;
    Settings::SettingsDialog* _optionsDialog;
    QPointer<AnnotationDialog::Dialog> _annotationDialog;
    Q3WidgetStack* _stack;
    QWidget* _welcome;
    QTimer* _autoSaveTimer;
    Browser::BrowserWidget* _browser;
    KTipDialog* _tipDialog;
    DeleteDialog* _deleteDialog;
    DirtyIndicator* _dirtyIndicator;
    QLabel* _lockedIndicator;
    KAction* _lock;
    KAction* _unlock;
    KAction* _setDefaultPos;
    KAction* _setDefaultNeg;
    KAction* _jumpToContext;
    HTMLGenerator::HTMLDialog* _htmlDialog;
    KAction* _configOneAtATime;
    KAction* _configAllSimultaniously;
    KAction* _createImageStack;
    KAction* _unStackImages;
    KAction* _setStackHead;
    KAction* _view;
    KAction* _rotLeft;
    KAction* _rotRight;
    KAction* _sortByDateAndTime;
    KAction* _viewInNewWindow;
    KActionMenu* _viewMenu;
    KToggleAction* _smallListView;
    KToggleAction* _largeListView;
    KToggleAction* _smallIconView;
    KToggleAction* _largeIconView;
    KAction* _generateHtml;
    KAction* _cut;
    KAction* _paste;
    KAction* _deleteSelected;
    KAction* _limitToMarked;
    KAction* _selectAll;
    KAction* _runSlideShow;
    KAction* _runRandomSlideShow;
    Plugins::Interface* _pluginInterface;
    KAction* _showExifDialog;
#ifdef HASKIPI
    KIPI::PluginLoader* _pluginLoader;
#endif
    KAction* _recreateThumbnails;
    TokenEditor* _tokenEditor;
    DateBar::DateBarWidget* _dateBar;
    bool _hasLoadedPlugins;
    ImageCounter* _partial;
    BreadcrumbViewer* _pathIndicator;
};

}

#endif /* MAINVIEW_H */

