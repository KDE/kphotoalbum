/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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

#ifndef MAINWINDOW_WINDOW_H
#define MAINWINDOW_WINDOW_H
#include <config-kpa-marble.h>
#ifdef HAVE_MARBLE
#include <Browser/PositionBrowserWidget.h>
#endif
#include <DB/Category.h>
#include <DB/FileNameList.h>
#include <DB/ImageSearchInfo.h>
#include <DB/UIDelegate.h>
#include <ThumbnailView/enums.h>

#include <KXmlGuiWindow>
#include <QList>
#include <QPointer>
#include <QUrl>
#include <config-kpa-kipi.h>

class QAction;
class QCloseEvent;
class QContextMenuEvent;
class QFrame;
class QLabel;
class QMoveEvent;
class QResizeEvent;
class QStackedWidget;
class QTimer;

class KActionMenu;
class KTipDialog;
class KToggleAction;

#ifdef HASKIPI
namespace KIPI
{
class PluginLoader;
}
#endif

namespace AnnotationDialog
{
class Dialog;
}
namespace Browser
{
class BrowserWidget;
class BreadcrumbList;
}
namespace DateBar
{
class DateBarWidget;
}
namespace DB
{
class ImageInfoList;
}
namespace HTMLGenerator
{
class HTMLDialog;
}
namespace Plugins
{
class Interface;
}
namespace Settings
{
class SettingsDialog;
}
namespace ThumbnailView
{
class ThumbnailFacade;
}

class BreadcrumbViewer;

namespace MainWindow
{
class DeleteDialog;
class StatusBar;
class TokenEditor;

class Window : public KXmlGuiWindow, public DB::UIDelegate
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent);
    ~Window() override;
    static void configureImages(const DB::ImageInfoList &list, bool oneAtATime);
    static Window *theMainWindow();
    DB::FileNameList selected(ThumbnailView::SelectionMode mode = ThumbnailView::ExpandCollapsedStacks) const;
    DB::ImageSearchInfo currentContext();
    QString currentBrowseCategory() const;
    void setStackHead(const DB::FileName &image);
    void setHistogramVisibilty(bool visible) const;
    bool dbIsDirty() const;
#ifdef HAVE_MARBLE
    void showPositionBrowser();
    Browser::PositionBrowserWidget *positionBrowserWidget();
#endif

    // implement UI delegate interface
    // Note(jzarl): we just could create a UIDelegate class that takes a QWidget,
    // implementing the same messageParent approach that we took before.
    // For now, I don't see anything wrong with directly implementing the interface instead.
    // I may change my mind later and I'm ready to convinced of the errors of my way, though...
    DB::UserFeedback askWarningContinueCancel(const QString &msg, const QString &title, const QString &dialogId) override;
    DB::UserFeedback askQuestionYesNo(const QString &msg, const QString &title, const QString &dialogId) override;
    void showInformation(const QString &msg, const QString &title, const QString &dialogId) override;
    void showSorry(const QString &msg, const QString &title, const QString &) override;
    void showError(const QString &msg, const QString &title, const QString &) override;
    bool isDialogDisabled(const QString &dialogId) override;

public slots:
    void showThumbNails(const DB::FileNameList &items);
    void loadKipiPlugins();
    void reloadThumbnails(ThumbnailView::SelectionUpdateMethod method = ThumbnailView::MaintainSelection);
    void runDemo();
    void slotImageRotated(const DB::FileName &fileName);
    void slotSave();

protected slots:
    void showThumbNails();
    bool slotExit();
    void slotOptions();
    void slotConfigureAllImages();
    void slotConfigureImagesOneAtATime();
    void slotCreateImageStack();
    void slotUnStackImages();
    void slotSetStackHead();
    void slotCopySelectedURLs();
    void slotPasteInformation();
    void slotDeleteSelected();
    void slotReReadExifInfo();
    void slotAutoStackImages();
    void slotSearch();
    void slotView(bool reuse = true, bool slideShow = false, bool random = false);
    void slotViewNewWindow();
    void slotSortByDateAndTime();
    void slotSortAllByDateAndTime();
    void slotLimitToSelected();
    void slotExportToHTML();
    void slotAutoSave();
    void showBrowser();
    void slotOptionGroupChanged();
    void showTipOfDay();
    void lockToDefaultScope();
    void setDefaultScopePositive();
    void setDefaultScopeNegative();
    void unlockFromDefaultScope();
    void changePassword();
    void slotConfigureKeyBindings();
    void slotSetFileName(const DB::FileName &);
    void updateContextMenuFromSelectionSize(int selectionSize);
    void slotUpdateViewMenu(DB::Category::ViewType);
    void slotShowNotOnDisk();
    void slotBuildThumbnails();
    void slotBuildThumbnailsIfWanted();
    void slotRunSlideShow();
    void slotRunRandomizedSlideShow();
    void slotConfigureToolbars();
    void slotNewToolbarConfig();
    void slotImport();
    void slotExport();
    void delayedInit();
    void slotReenableMessages();
    void slotImagesChanged(const QList<QUrl> &);
    void slotSelectionChanged(int count);
    void plug();
    void slotRemoveTokens();
    void slotShowListOfFiles();
    void updateDateBar(const Browser::BreadcrumbList &);
    void updateDateBar();
    void slotShowImagesWithInvalidDate();
    void slotShowImagesWithChangedMD5Sum();
    void showDateBarTip(const QString &);
    void slotJumpToContext();
    void setDateRange(const DB::ImageDate &);
    void clearDateRange();
    void startAutoSaveTimer();
    void slotRecalcCheckSums();
    void slotShowExifInfo();
    void showFeatures();
    void showImage(const DB::FileName &fileName);
    void slotOrderIncr();
    void slotOrderDecr();
    void slotRotateSelectedLeft();
    void slotRotateSelectedRight();
    void rotateSelected(int angle);
    void showVideos();
    void slotStatistics();
    void slotRecreateExifDB();
    void useNextVideoThumbnail();
    void usePreviousVideoThumbnail();
    void mergeDuplicates();
    void slotThumbnailSizeChanged();
    void slotMarkUntagged();

protected:
    void configureImages(bool oneAtATime);
    QString welcome();
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent *) override;
    void moveEvent(QMoveEvent *) override;
    void setupMenuBar();
    void createAnnotationDialog();
    bool load();
    void contextMenuEvent(QContextMenuEvent *e) override;
    void setLocked(bool b, bool force, bool recount = true);
    void configImages(const DB::ImageInfoList &list, bool oneAtATime);
    void updateStates(bool thumbNailView);
    DB::FileNameList selectedOnDisk();
    void setupPluginMenu();
    void launchViewer(const DB::FileNameList &mediaList, bool reuse, bool slideShow, bool random);
    void setupStatusBar();
    void setPluginMenuState(const char *name, const QList<QAction *> &actions);
    void createSearchBar();
    void executeStartupActions();
    void checkIfVideoThumbnailerIsInstalled();
    bool anyVideosSelected() const;

private:
    static Window *s_instance;

    ThumbnailView::ThumbnailFacade *m_thumbnailView;
    Settings::SettingsDialog *m_settingsDialog;
    QPointer<AnnotationDialog::Dialog> m_annotationDialog;
    QStackedWidget *m_stack;
    QTimer *m_autoSaveTimer;
    Browser::BrowserWidget *m_browser;
    DeleteDialog *m_deleteDialog;
    QAction *m_lock;
    QAction *m_unlock;
    QAction *m_setDefaultPos;
    QAction *m_setDefaultNeg;
    QAction *m_jumpToContext;
    HTMLGenerator::HTMLDialog *m_htmlDialog;
    QAction *m_configOneAtATime;
    QAction *m_configAllSimultaniously;
    QAction *m_createImageStack;
    QAction *m_unStackImages;
    QAction *m_setStackHead;
    QAction *m_view;
    QAction *m_rotLeft;
    QAction *m_rotRight;
    QAction *m_sortByDateAndTime;
    QAction *m_sortAllByDateAndTime;
    QAction *m_AutoStackImages;
    QAction *m_viewInNewWindow;
    KActionMenu *m_viewMenu;
    KToggleAction *m_smallListView;
    KToggleAction *m_largeListView;
    KToggleAction *m_largeIconView;
    QAction *m_generateHtml;
    QAction *m_copy;
    QAction *m_paste;
    QAction *m_deleteSelected;
    QAction *m_limitToMarked;
    QAction *m_selectAll;
    QAction *m_clearSelection;
    QAction *m_runSlideShow;
    QAction *m_runRandomSlideShow;
    Plugins::Interface *m_pluginInterface;
    QAction *m_showExifDialog;
#ifdef HASKIPI
    KIPI::PluginLoader *m_pluginLoader;
#endif
    QAction *m_recreateThumbnails;
    QAction *m_useNextVideoThumbnail;
    QAction *m_usePreviousVideoThumbnail;
    QAction *m_markUntagged;
    TokenEditor *m_tokenEditor;
    DateBar::DateBarWidget *m_dateBar;
    QFrame *m_dateBarLine;
    bool m_hasLoadedKipiPlugins;
    QMap<Qt::Key, QPair<QString, QString>> m_viewerInputMacros;
    MainWindow::StatusBar *m_statusBar;
    QString m_lastTarget;
#ifdef HAVE_MARBLE
    Browser::PositionBrowserWidget *m_positionBrowser;
#endif
};

}

#endif /* MAINWINDOW_WINDOW_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
