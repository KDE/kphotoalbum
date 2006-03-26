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

#ifndef MAINVIEW_H
#define MAINVIEW_H
class TokenEditor;
class OptionsDialog;
class QWidgetStack;
class ImageCounter;
class QTimer;
class KTipDialog;
class DeleteDialog;
class ReadInfoDialog;
class QLabel;
class HTMLExportDialog;
class KActionMenu;
class KRadioAction;

namespace Plugins
{
    class Interface;
}

namespace ThumbnailView
{
    class ThumbnailView;
}

namespace Browser
{
    class Browser;
}

namespace AnnotationDialog
{
    class AnnotationDialog;
}

#include "imageinfolist.h"
#include <qdict.h>
#include <kmainwindow.h>
#include "options.h"
#include <kurl.h>
#include "category.h"
#include <config.h>
#ifdef HASKIPI
#  include <libkipi/pluginloader.h>
#endif

namespace DateBar
{
    class DateBar;
}

class MainView :public KMainWindow
{
    Q_OBJECT

public:
    MainView( QWidget* parent,  const char* name = 0 );
    static void configureImages( const ImageInfoList& list, bool oneAtATime );
    static MainView* theMainView();
    QStringList selected( bool keepSortOrderOfDatabase = false );
    ImageSearchInfo currentContext();
    QString currentBrowseCategory() const;

public slots:
    void showThumbNails( const QStringList& list );
    void loadPlugins();

protected slots:
    void showThumbNails();
    bool slotExit();
    void slotOptions();
    void slotConfigureAllImages();
    void slotConfigureImagesOneAtATime();
    void slotSave();
    void slotDeleteSelected();
    void slotReReadExifInfo();
    void slotSearch();
    void slotView( bool reuse = false, bool slideShow = false, bool random = false );
    void slotViewNewWindow();
    void slotSortByDateAndTime();
    void slotChanges();
    void slotLimitToSelected();
    void slotExportToHTML();
    void slotAutoSave();
    void showBrowser();
    void slotOptionGroupChanged();
    void showTipOfDay();
    void pathChanged( const QString& );
    void runDemo();
    void runSurvey();
    void lockToDefaultScope();
    void setDefaultScopePositive();
    void setDefaultScopeNegative();
    void unlockFromDefaultScope();
    void changePassword();
    void slotConfigureKeyBindings();
    void slotSetFileName( const QString& );
    void slotThumbNailSelectionChanged();
    void reloadThumbnails(bool flushCache);
    void reloadThumbnailsAndFlushCache();
    void slotUpdateViewMenu( Category::ViewSize, Category::ViewType );
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
    void slotSelectionChanged();
    void plug();
    void slotRemoveTokens();
    void updateDateBar( const QString& );
    void updateDateBar();
    void slotShowImagesWithInvalidDate();
    void showDateBarTip( const QString& );
    void slotJumpToContext();
    void setDateRange( const ImageDate& );
    void clearDateRange();
    void deleteAnnotationDialog();
    void startAutoSaveTimer();
    void convertBackend();
    void slotRecalcCheckSums();
    void slotShowExifInfo();
    void showFeatures();
    void showImage( const QString& fileName );
    void slotOrderIncr();
    void slotOrderDecr();

protected:
    void configureImages( bool oneAtATime );
    QString welcome();
    virtual void closeEvent( QCloseEvent* e );
    virtual void resizeEvent( QResizeEvent* );
    virtual void moveEvent ( QMoveEvent * );
    void setupMenuBar();
    void createAnnotationDialog();
    void load();
    virtual void contextMenuEvent( QContextMenuEvent* e );
    void setDirty( bool b );
    void setLocked( bool b );
    void configImages( const ImageInfoList& list, bool oneAtATime );
    void updateStates( bool thumbNailView );
    QStringList selectedOnDisk();
    void possibleRunSuvey();
    void setupPluginMenu();

private:
    static MainView* _instance;

    ThumbnailView::ThumbnailView* _thumbnailView;
    OptionsDialog* _optionsDialog;
    QGuardedPtr<AnnotationDialog::AnnotationDialog> _annotationDialog;
    bool _dirty;
    bool _autoSaveDirty; // We do not want to continue autosaving the same date
    QWidgetStack* _stack;
    QWidget* _welcome;
    QTimer* _autoSaveTimer;
    Browser::Browser* _browser;
    KTipDialog* _tipDialog;
    DeleteDialog* _deleteDialog;
    QLabel* _dirtyIndicator;
    QLabel* _lockedIndicator;
    KAction* _lock;
    KAction* _unlock;
    KAction* _setDefaultPos;
    KAction* _setDefaultNeg;
    KAction* _jumpToContext;
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
    Plugins::Interface* _pluginInterface;
    KAction* _showExifDialog;

#ifdef HASKIPI
    KIPI::PluginLoader* _pluginLoader;
#endif
    TokenEditor* _tokenEditor;
    DateBar::DateBar* _dateBar;
    bool _hasLoadedPlugins;
    ImageCounter* _partial;
};


#endif /* MAINVIEW_H */

