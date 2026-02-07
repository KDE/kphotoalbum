// SPDX-FileCopyrightText: 2003 - 2014 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2003 Stephan Binner <binner@kde.org>
// SPDX-FileCopyrightText: 2005 - 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2006 - 2010 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007 - 2009 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2007 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2008 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2010 - 2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2010 Wes Hardaker <kpa@capturedonearth.com>
// SPDX-FileCopyrightText: 2011 Andreas Neustifter <andreas.neustifter@gmail.com>
// SPDX-FileCopyrightText: 2012 - 2026 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2013 Dominik Broj <broj.dominik@gmail.com>
// SPDX-FileCopyrightText: 2014 - 2017 Robert Krawitz <rlk@alum.mit.edu>
// SPDX-FileCopyrightText: 2015 - 2022 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MAINWINDOW_WINDOW_H
#define MAINWINDOW_WINDOW_H
#include <DB/Category.h>
#include <DB/search/ImageSearchInfo.h>
#include <MainWindow/CopyLinkEngine.h>
#include <ThumbnailView/enums.h>
#include <kpabase/FileNameList.h>
#include <kpabase/UIDelegate.h>
#include <kpabase/config-kpa-marble.h>
#include <kpabase/config-kpa-plugins.h>

#include <KXmlGuiWindow>
#include <QList>
#include <QPointer>
#include <QUrl>

class QAction;
class QContextMenuEvent;
class QFrame;
class QLabel;
class QStackedWidget;
class QTimer;

class KActionMenu;
class KTipDialog;
class KToggleAction;

#ifdef HAVE_MARBLE

namespace Viewer
{
class ViewerWidget;
}
namespace Map
{
class MapView;
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
namespace ImageManager
{
class ThumbnailCache;
class VideoThumbnailCache;
}
namespace Settings
{
class SettingsDialog;
}
namespace ThumbnailView
{
class ThumbnailFacade;
class FilterWidget;
}

class BreadcrumbViewer;

namespace MainWindow
{

class SearchBar;
class DeleteDialog;
class StatusBar;
class TokenEditor;
class CopyLinkEngine;

class Window : public KXmlGuiWindow, public DB::UIDelegate
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent);
    ~Window() override;
    static void configureImages(const DB::ImageInfoList &list, bool oneAtATime);
    static Window *theMainWindow();

    ImageManager::ThumbnailCache *thumbnailCache() const;
    ImageManager::VideoThumbnailCache *videoThumbnailCache() const;

    DB::FileNameList selected(ThumbnailView::SelectionMode mode = ThumbnailView::ExpandCollapsedStacks) const;
    DB::ImageSearchInfo currentContext();
    QString currentBrowseCategory() const;
    void setStackHead(const DB::FileName &image);
    void setHistogramVisibilty(bool visible) const;
    bool dbIsDirty() const;
#ifdef HAVE_MARBLE
    void showPositionBrowser();
    Map::MapView *positionBrowserWidget();
#endif

    // implement UI delegate interface
    // Note(jzarl): we just could create a UIDelegate class that takes a QWidget,
    // implementing the same messageParent approach that we took before.
    // For now, I don't see anything wrong with directly implementing the interface instead.
    // I may change my mind later and I'm ready to convinced of the errors of my way, though...
    DB::UserFeedback askWarningContinueCancel(const QString &msg, const QString &title, const QString &dialogId) override;
    DB::UserFeedback askQuestionYesNo(const QString &msg, const QString &title, const QString &dialogId) override;
    void showInformation(const QString &msg, const QString &title, const QString &dialogId) override;
    void showError(const QString &msg, const QString &title, const QString &) override;
    bool isDialogDisabled(const QString &dialogId) override;

public Q_SLOTS:
    void showThumbNails(const DB::FileNameList &items);
    void reloadThumbnails(ThumbnailView::SelectionUpdateMethod method = ThumbnailView::MaintainSelection);
    void runDemo();
    void slotImageRotated(const DB::FileName &fileName);
    void slotSave();

protected Q_SLOTS:
    void showThumbNails();
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
    // FIXME(jzarl): improve this function signature:
    void slotView(bool reuse, bool slideShow = false, bool random = false);
    void slotView();
    void slotViewNewWindow();
    void slotSortByDateAndTime();
    void slotSortAllByDateAndTime();
    void slotLimitToSelected();
    void slotExportToHTML();
    void slotAutoSave();
    void showBrowser();
    void slotOptionGroupChanged();
    void slotFilterChanged();
    void lockToDefaultScope();
    void setDefaultScopePositive();
    void setDefaultScopeNegative();
    void unlockFromDefaultScope();
    void configureShortcuts();
    void slotSetFileName(const DB::FileName &);
    void updateContextMenuFromSelectionSize(int selectionSize);
    void slotUpdateViewMenu(DB::Category::ViewType);
    void slotShowNotOnDisk();
    void slotBuildThumbnails();
    void slotBuildThumbnailsIfWanted();
    void slotRunSlideShow();
    void slotRunRandomizedSlideShow();
    void slotImport();
    void slotExport();
    void delayedInit();
    void slotReenableMessages();
    void slotImagesChanged(const QList<QUrl> &);
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
    void slotRescan();
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
    void triggerCopyLinkAction(CopyLinkEngine::Action action);

protected:
    void configureImages(bool oneAtATime);
    QString welcome();
    bool event(QEvent *event) override;
    void setupMenuBar();
    void createAnnotationDialog();
    bool load();
    void contextMenuEvent(QContextMenuEvent *e) override;
    void setLocked(bool b, bool force, bool recount = true);
    void configImages(const DB::ImageInfoList &list, bool oneAtATime);
    DB::FileNameList selectedOnDisk();
    void setupPluginMenu();
    void launchViewer(const DB::FileNameList &mediaList, bool reuse, bool slideShow, bool random);
    void setupStatusBar();
    void setPluginMenuState(const char *name, const QList<QAction *> &actions);
    void createSearchBar();
    void executeStartupActions();
    void checkIfVideoThumbnailerIsInstalled();
    bool anyVideosSelected() const;
    bool queryClose() override;

private:
    static Window *s_instance;

    ImageManager::ThumbnailCache *m_thumbnailCache;
    ImageManager::VideoThumbnailCache *m_videoThumbnailCache;
    ThumbnailView::ThumbnailFacade *m_thumbnailView;
    Settings::SettingsDialog *m_settingsDialog;
    QPointer<AnnotationDialog::Dialog> m_annotationDialog;
    QStackedWidget *m_stack;
    QTimer *m_autoSaveTimer;
    Browser::BrowserWidget *m_browser;
    SearchBar *m_searchBar;
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
    QAction *m_copyAction;
    QAction *m_linkAction;
    KActionMenu *m_viewMenu;
    KToggleAction *m_smallListView;
    KToggleAction *m_largeListView;
    KToggleAction *m_largeIconView;
    KToggleAction *m_sortViewNaturally;
    KActionMenu *m_colorSchemeMenu;
    QAction *m_generateHtml;
    QAction *m_copy;
    QAction *m_paste;
    QAction *m_deleteSelected;
    QAction *m_limitToMarked;
    QAction *m_selectAll;
    QAction *m_clearSelection;
    QAction *m_runSlideShow;
    QAction *m_runRandomSlideShow;
    QAction *m_showExifDialog;
    QAction *m_recreateThumbnails;
    QAction *m_useNextVideoThumbnail;
    QAction *m_usePreviousVideoThumbnail;
    QAction *m_markUntagged;
    TokenEditor *m_tokenEditor;
    DateBar::DateBarWidget *m_dateBar;
    QFrame *m_dateBarLine;
    MainWindow::StatusBar *m_statusBar;
    QString m_lastTarget;
#ifdef HAVE_MARBLE
    Map::MapView *m_positionBrowser;
#endif
    ThumbnailView::FilterWidget *m_filterWidget;
    CopyLinkEngine *m_copyLinkEngine;

    QList<Viewer::ViewerWidget *> m_viewers; ///< A list of all currently open full featured ViewerWidgets
};

}

#endif /* MAINWINDOW_WINDOW_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
