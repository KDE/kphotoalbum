// SPDX-FileCopyrightText: 2003 David Faure <faure@kde.org>
// SPDX-FileCopyrightText: 2003-2005 Stephan Binner <binner@kde.org>
// SPDX-FileCopyrightText: 2003-2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2003-2023 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2004 Marc Mutz <mutz@kde.org>
// SPDX-FileCopyrightText: 2006-2010 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007 Shawn Willden <shawn-kimdaba@willden.org>
// SPDX-FileCopyrightText: 2007 Thiago Macieira <thiago@kde.org>
// SPDX-FileCopyrightText: 2007-2008 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2007-2010 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2008 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2008 Luboš Luňák <l.lunak@kde.org>
// SPDX-FileCopyrightText: 2009, 2022 Yuri Chornoivan <yurchor@ukr.net>
// SPDX-FileCopyrightText: 2009-2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2010 Wes Hardaker <kpa@capturedonearth.com>
// SPDX-FileCopyrightText: 2013-2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2015-2020 Robert Krawitz <rlk@alum.mit.edu>
// SPDX-FileCopyrightText: 2018 Antoni Bella Pérez <antonibella5@yahoo.com>
// SPDX-FileCopyrightText: 2022 Friedrich W. H. Kossebau <kossebau@kde.org>
// SPDX-FileCopyrightText: 2014-2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ViewerWidget.h"
#include <config-kpa-videobackends.h>

#include "CategoryImageConfig.h"
#include "CursorVisibilityHandler.h"
#include "ImageDisplay.h"
#include "InfoBox.h"
#include "Logging.h"

#if Phonon4Qt5_FOUND
#include "PhononDisplay.h"
#endif

#if QtAV_FOUND
#include "QtAVDisplay.h"
#endif

#include "TaggedArea.h"
#include "TextDisplay.h"
#include "TransientDisplay.h"

#if LIBVLC_FOUND
#include "VLCDisplay.h"
#endif

#include "AnnotationHandler.h"
#include "VideoDisplay.h"
#include "VideoShooter.h"
#include "VisibleOptionsMenu.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <Exif/InfoDialog.h>
#include <MainWindow/CategoryImagePopup.h>
#include <MainWindow/DeleteDialog.h>
#include <MainWindow/DirtyIndicator.h>
#include <MainWindow/ExternalPopup.h>
#include <MainWindow/Window.h>
#include <Settings/VideoPlayerSelectorDialog.h>
#include <Utilities/DescriptionUtil.h>
#include <kpabase/FileExtensions.h>
#include <kpabase/SettingsData.h>
#include <kpathumbnails/ThumbnailCache.h>

#include <KActionCollection>
#include <KColorScheme>
#include <KIO/CopyJob>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QList>
#include <QPushButton>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QTimeLine>
#include <QTimer>
#include <QWheelEvent>
#include <qglobal.h>

#include <QDesktopServices>
#include <QInputDialog>
#include <QMetaEnum>

#include <functional>
#include <utility>

using namespace std::chrono_literals;

Viewer::ViewerWidget *Viewer::ViewerWidget::s_latest = nullptr;

Viewer::ViewerWidget *Viewer::ViewerWidget::latest()
{
    return s_latest;
}

// Notice the parent is zero to allow other windows to come on top of it.
Viewer::ViewerWidget::ViewerWidget(UsageType type)
    : QStackedWidget(nullptr)
    , m_crashSentinel(QString::fromUtf8("videoBackend"))
    , m_screenSaverCookie(-1)
    , m_current(0)
    , m_popup(nullptr)
    , m_showingFullScreen(false)
    , m_forward(true)
    , m_isRunningSlideShow(false)
    , m_videoPlayerStoppedManually(false)
    , m_type(type)
    , m_copyLinkEngine(nullptr)
    , m_annotationHandler(new AnnotationHandler(this))
{
    if (type == UsageType::FullFeaturedViewer) {
        setWindowFlags(Qt::Window);
        setAttribute(Qt::WA_DeleteOnClose);
        s_latest = this;
    }

    m_display = m_imageDisplay = new ImageDisplay(this);
    addWidget(m_imageDisplay);
    m_cursorHandlerForImageDisplay = new CursorVisibilityHandler(m_imageDisplay);

    m_textDisplay = new TextDisplay(this);
    addWidget(m_textDisplay);

    createVideoViewer();

    connect(m_imageDisplay, &ImageDisplay::possibleChange, this, &ViewerWidget::updateCategoryConfig);
    connect(m_imageDisplay, &ImageDisplay::imageReady, this, &ViewerWidget::updateInfoBox);
    connect(m_imageDisplay, &ImageDisplay::imageZoomCaptionChanged, this, &ViewerWidget::setCaptionWithDetail);
    connect(m_imageDisplay, &ImageDisplay::viewGeometryChanged, this, &ViewerWidget::remapAreas);

    // This must not be added to the layout, as it is standing on top of
    // the ImageDisplay
    m_infoBox = new InfoBox(this);
    m_infoBox->hide();

    setupContextMenu();

    m_slideShowTimer = new QTimer(this);
    m_slideShowTimer->setSingleShot(true);
    m_slideShowPause = Settings::SettingsData::instance()->slideShowInterval() * 1000;
    connect(m_slideShowTimer, &QTimer::timeout, this, &ViewerWidget::slotSlideShowNextFromTimer);
    m_transientDisplay = new TransientDisplay(this);
    m_transientDisplay->hide();

    setFocusPolicy(Qt::StrongFocus);

    QTimer::singleShot(2000, this, &ViewerWidget::test);

    connect(DB::ImageDB::instance(), &DB::ImageDB::imagesDeleted, this, &ViewerWidget::slotRemoveDeletedImages);

    updatePalette();
    connect(Settings::SettingsData::instance(), &Settings::SettingsData::colorSchemeChanged, this, &ViewerWidget::updatePalette);

    connect(m_annotationHandler, &AnnotationHandler::requestToggleCategory,
            this, &Viewer::ViewerWidget::toggleTag);
    connect(m_annotationHandler, &AnnotationHandler::requestHelp,
            this, &Viewer::ViewerWidget::showAnnotationHelp);
}

void Viewer::ViewerWidget::setupContextMenu()
{
    m_popup = new QMenu(this);
    m_actions = new KActionCollection(this);

    // we make unused features invisible to avoid uninitialized values all over the class
    const bool showFullFeatures = m_type == UsageType::FullFeaturedViewer;

    createAnnotationMenu();
    createSlideShowMenu();
    createZoomMenu();
    createRotateMenu();
    createSkipMenu();
    createShowContextMenu();
    createInvokeExternalMenu();
    createVideoMenu();
    createCategoryImageMenu();
    createFilterMenu();

    m_setStackHead = m_actions->addAction(QString::fromLatin1("viewer-set-stack-head"), this, &ViewerWidget::slotSetStackHead);
    m_setStackHead->setText(i18nc("@action:inmenu", "Set as First Image in Stack"));
    m_setStackHead->setVisible(showFullFeatures);
    m_actions->setDefaultShortcut(m_setStackHead, QKeySequence(Qt::CTRL, Qt::Key_4));
    m_popup->addAction(m_setStackHead);

    m_showExifViewer = m_actions->addAction(QString::fromLatin1("viewer-show-exif-viewer"), this, &ViewerWidget::showExifViewer);
    m_showExifViewer->setText(i18n("Show Exif Info and file metadata"));
    m_popup->addAction(m_showExifViewer);

    m_popup->addSeparator();

    m_copyToAction = m_actions->addAction(QStringLiteral("viewer-copy-to"), this, std::bind(&ViewerWidget::triggerCopyLinkAction, this, MainWindow::CopyLinkEngine::Copy));
    m_copyToAction->setText(i18nc("@action:inmenu", "Copy image to ..."));
    m_copyToAction->setVisible(showFullFeatures);
    m_actions->setDefaultShortcut(m_copyToAction, Qt::Key_F7);
    m_popup->addAction(m_copyToAction);

    m_linkToAction = m_actions->addAction(QStringLiteral("viewer-link-to"), this, std::bind(&ViewerWidget::triggerCopyLinkAction, this, MainWindow::CopyLinkEngine::Link));
    m_linkToAction->setText(i18nc("@action:inmenu", "Link image to ..."));
    m_linkToAction->setVisible(showFullFeatures);
    m_actions->setDefaultShortcut(m_linkToAction, QKeySequence(Qt::SHIFT, Qt::Key_F7));
    m_popup->addAction(m_linkToAction);

    m_popup->addSeparator();

    auto action = m_actions->addAction(QString::fromLatin1("viewer-close"), this, &ViewerWidget::close);
    action->setText(i18nc("@action:inmenu", "Close"));
    action->setShortcut(Qt::Key_Escape);
    action->setVisible(showFullFeatures);
    m_actions->setShortcutsConfigurable(action, false);
    m_popup->addAction(action);

    m_actions->readSettings();

    const auto actions = m_actions->actions();
    for (QAction *action : actions) {
        action->setShortcutContext(Qt::WindowShortcut);
        addAction(action);
    }
}

void Viewer::ViewerWidget::createShowContextMenu()
{
    VisibleOptionsMenu *menu = new VisibleOptionsMenu(this, m_actions);
    connect(menu, &VisibleOptionsMenu::visibleOptionsChanged, this, &ViewerWidget::updateInfoBox);
    const bool showFullFeatures = m_type == UsageType::FullFeaturedViewer;
    m_popup->addMenu(menu)->setVisible(showFullFeatures);
}

void Viewer::ViewerWidget::inhibitScreenSaver(bool inhibit)
{
    QDBusMessage message;
    if (inhibit) {
        message = QDBusMessage::createMethodCall(QString::fromLatin1("org.freedesktop.ScreenSaver"), QString::fromLatin1("/ScreenSaver"),
                                                 QString::fromLatin1("org.freedesktop.ScreenSaver"), QString::fromLatin1("Inhibit"));

        message << QString(QString::fromLatin1("KPhotoAlbum"));
        message << QString(QString::fromLatin1("Giving a slideshow"));
        QDBusMessage reply = QDBusConnection::sessionBus().call(message);
        if (reply.type() == QDBusMessage::ReplyMessage)
            m_screenSaverCookie = reply.arguments().constFirst().toInt();
    } else {
        if (m_screenSaverCookie != -1) {
            message = QDBusMessage::createMethodCall(QString::fromLatin1("org.freedesktop.ScreenSaver"), QString::fromLatin1("/ScreenSaver"),
                                                     QString::fromLatin1("org.freedesktop.ScreenSaver"), QString::fromLatin1("UnInhibit"));
            message << (uint)m_screenSaverCookie;
            QDBusConnection::sessionBus().send(message);
            m_screenSaverCookie = -1;
        }
    }
}

DB::FileName Viewer::ViewerWidget::currentFileName() const
{
    return m_list.value(m_current);
}

void Viewer::ViewerWidget::createInvokeExternalMenu()
{
    m_externalPopup = new MainWindow::ExternalPopup(m_popup);
    const bool showFullFeatures = m_type == UsageType::FullFeaturedViewer;
    m_popup->addMenu(m_externalPopup)->setVisible(showFullFeatures);
    connect(m_externalPopup, &MainWindow::ExternalPopup::aboutToShow, this, &ViewerWidget::populateExternalPopup);
}

void Viewer::ViewerWidget::createRotateMenu()
{
    m_rotateMenu = new QMenu(m_popup);
    m_rotateMenu->setTitle(i18nc("@title:inmenu", "Rotate"));

    auto addRotateAction = [this](const QString &title, int angle, const QKeySequence &shortcut, const QString &actionName) {
        auto *action = new QAction(title);
        connect(action, &QAction::triggered, [this, angle] { rotate(angle); });
        action->setShortcut(shortcut);
        m_actions->setShortcutsConfigurable(action, false);
        m_actions->addAction(actionName, action);
        m_rotateMenu->addAction(action);
    };

    addRotateAction(i18nc("@action:inmenu", "Rotate clockwise"), 90, Qt::Key_9, QString::fromLatin1("viewer-rotate90"));
    addRotateAction(i18nc("@action:inmenu", "Flip Over"), 180, Qt::Key_8, QString::fromLatin1("viewer-rotate180"));
    addRotateAction(i18nc("@action:inmenu", "Rotate counterclockwise"), 270, Qt::Key_7, QString::fromLatin1("viewer-rotate270"));
    const bool showFullFeatures = m_type == UsageType::FullFeaturedViewer;
    // hide entries of hidden menus so that they can't be triggered via shortcut:
    for (auto &action : m_rotateMenu->actions())
        action->setVisible(showFullFeatures);
    m_popup->addMenu(m_rotateMenu)->setVisible(showFullFeatures);
}

void Viewer::ViewerWidget::createSkipMenu()
{
    QMenu *popup = new QMenu(m_popup);
    popup->setTitle(i18nc("@title:inmenu As in 'skip 2 images'", "Skip"));

    QAction *action = m_actions->addAction(QString::fromLatin1("viewer-home"), this, &ViewerWidget::showFirst);
    action->setText(i18nc("@action:inmenu Go to first image", "First"));
    action->setShortcut(Qt::Key_Home);
    m_actions->setShortcutsConfigurable(action, false);
    popup->addAction(action);
    m_backwardActions.append(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-end"), this, &ViewerWidget::showLast);
    action->setText(i18nc("@action:inmenu Go to last image", "Last"));
    action->setShortcut(Qt::Key_End);
    m_actions->setShortcutsConfigurable(action, false);
    popup->addAction(action);
    m_forwardActions.append(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-next"), this, &ViewerWidget::showNext);
    action->setText(i18nc("@action:inmenu", "Show Next"));
    action->setShortcuts(QList<QKeySequence>() << Qt::Key_PageDown << Qt::Key_Space);
    popup->addAction(action);
    m_forwardActions.append(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-next-10"), this, &ViewerWidget::showNext10);
    action->setText(i18nc("@action:inmenu", "Skip 10 Forward"));
    m_actions->setDefaultShortcut(action, QKeySequence(Qt::CTRL, Qt::Key_PageDown));
    popup->addAction(action);
    m_forwardActions.append(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-next-100"), this, &ViewerWidget::showNext100);
    action->setText(i18nc("@action:inmenu", "Skip 100 Forward"));
    m_actions->setDefaultShortcut(action, QKeySequence(Qt::SHIFT, Qt::Key_PageDown));
    popup->addAction(action);
    m_forwardActions.append(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-next-1000"), this, &ViewerWidget::showNext1000);
    action->setText(i18nc("@action:inmenu", "Skip 1000 Forward"));
    m_actions->setDefaultShortcut(action, QKeySequence(Qt::CTRL, Qt::SHIFT, Qt::Key_PageDown));
    popup->addAction(action);
    m_forwardActions.append(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-prev"), this, &ViewerWidget::showPrev);
    action->setText(i18nc("@action:inmenu", "Show Previous"));
    action->setShortcuts(QList<QKeySequence>() << Qt::Key_PageUp << Qt::Key_Backspace);
    popup->addAction(action);
    m_backwardActions.append(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-prev-10"), this, &ViewerWidget::showPrev10);
    action->setText(i18nc("@action:inmenu", "Skip 10 Backward"));
    m_actions->setDefaultShortcut(action, QKeySequence(Qt::CTRL, Qt::Key_PageUp));
    popup->addAction(action);
    m_backwardActions.append(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-prev-100"), this, &ViewerWidget::showPrev100);
    action->setText(i18nc("@action:inmenu", "Skip 100 Backward"));
    m_actions->setDefaultShortcut(action, QKeySequence(Qt::SHIFT, Qt::Key_PageUp));
    popup->addAction(action);
    m_backwardActions.append(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-prev-1000"), this, &ViewerWidget::showPrev1000);
    action->setText(i18nc("@action:inmenu", "Skip 1000 Backward"));
    m_actions->setDefaultShortcut(action, QKeySequence(Qt::CTRL, Qt::SHIFT, Qt::Key_PageUp));
    popup->addAction(action);
    m_backwardActions.append(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-delete-current"), this, &ViewerWidget::deleteCurrent);
    action->setText(i18nc("@action:inmenu", "Delete Image"));
    m_actions->setDefaultShortcut(action, QKeySequence(Qt::CTRL, Qt::Key_Delete));
    popup->addAction(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-remove-current"), this, &ViewerWidget::removeCurrent);
    action->setText(i18nc("@action:inmenu", "Remove Image from Display List"));
    action->setShortcut(Qt::Key_Delete);
    m_actions->setShortcutsConfigurable(action, false);
    popup->addAction(action);

    const bool showFullFeatures = m_type == UsageType::FullFeaturedViewer;
    // hide entries of hidden menus so that they can't be triggered via shortcut:
    for (auto &action : popup->actions())
        action->setVisible(showFullFeatures);
    m_popup->addMenu(popup)->setVisible(showFullFeatures);
}

void Viewer::ViewerWidget::createZoomMenu()
{
    QMenu *popup = new QMenu(m_popup);
    popup->setTitle(i18nc("@action:inmenu", "Zoom"));

    // PENDING(blackie) Only for image display?
    QAction *action = m_actions->addAction(QString::fromLatin1("viewer-zoom-in"), this, &ViewerWidget::zoomIn);
    action->setText(i18nc("@action:inmenu", "Zoom In"));
    action->setShortcut(Qt::Key_Plus);
    m_actions->setShortcutsConfigurable(action, false);
    popup->addAction(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-zoom-out"), this, &ViewerWidget::zoomOut);
    action->setText(i18nc("@action:inmenu", "Zoom Out"));
    action->setShortcut(Qt::Key_Minus);
    m_actions->setShortcutsConfigurable(action, false);
    popup->addAction(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-zoom-full"), this, &ViewerWidget::zoomFull);
    action->setText(i18nc("@action:inmenu", "Full View"));
    action->setShortcut(Qt::Key_Period);
    m_actions->setShortcutsConfigurable(action, false);
    popup->addAction(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-zoom-pixel"), this, &ViewerWidget::zoomPixelForPixel);
    action->setText(i18nc("@action:inmenu", "Pixel for Pixel View"));
    action->setShortcut(Qt::Key_Equal);
    m_actions->setShortcutsConfigurable(action, false);
    popup->addAction(action);

    action = m_actions->addAction(QString::fromLatin1("viewer-toggle-fullscreen"), this, &ViewerWidget::toggleFullScreen);
    action->setText(i18nc("@action:inmenu", "Toggle Full Screen"));
    action->setShortcuts(QList<QKeySequence>() << Qt::Key_F11 << Qt::Key_Return);
    action->setVisible(m_type == UsageType::FullFeaturedViewer);
    popup->addAction(action);

    m_popup->addMenu(popup);
}

void Viewer::ViewerWidget::createSlideShowMenu()
{
    QMenu *popup = new QMenu(m_popup);
    popup->setTitle(i18nc("@title:inmenu", "Slideshow"));

    m_startStopSlideShow = m_actions->addAction(QString::fromLatin1("viewer-start-stop-slideshow"), this, &ViewerWidget::slotStartStopSlideShow);
    m_startStopSlideShow->setText(i18nc("@action:inmenu", "Run Slideshow"));
    m_actions->setDefaultShortcut(m_startStopSlideShow, QKeySequence(Qt::CTRL, Qt::Key_R));
    popup->addAction(m_startStopSlideShow);

    m_slideShowRunFaster = m_actions->addAction(QString::fromLatin1("viewer-run-faster"), this, &ViewerWidget::slotSlideShowFaster);
    m_slideShowRunFaster->setText(i18nc("@action:inmenu", "Run Faster"));
    m_actions->setDefaultShortcut(m_slideShowRunFaster, QKeySequence(Qt::CTRL, Qt::Key_Plus)); // if you change this, please update the info in Viewer::TransientDisplay
    popup->addAction(m_slideShowRunFaster);

    m_slideShowRunSlower = m_actions->addAction(QString::fromLatin1("viewer-run-slower"), this, &ViewerWidget::slotSlideShowSlower);
    m_slideShowRunSlower->setText(i18nc("@action:inmenu", "Run Slower"));
    m_actions->setDefaultShortcut(m_slideShowRunSlower, QKeySequence(Qt::CTRL, Qt::Key_Minus)); // if you change this, please update the info in Viewer::TransientDisplay
    popup->addAction(m_slideShowRunSlower);

    const bool showFullFeatures = m_type == UsageType::FullFeaturedViewer;
    // hide entries of hidden menus so that they can't be triggered via shortcut:
    for (auto &action : popup->actions())
        action->setVisible(showFullFeatures);
    m_popup->addMenu(popup)->setVisible(showFullFeatures);
}

void Viewer::ViewerWidget::load(const DB::FileNameList &list, int index)
{
    m_list = list;
    m_imageDisplay->setImageList(list);
    m_current = index;
    load();
}

void Viewer::ViewerWidget::load()
{
    const auto currentFile = currentFileName();
    if (currentFile.isNull())
        return;

    m_display->stop();
    const bool isReadable = QFileInfo(currentFile.absolute()).isReadable();
    const bool isVideo = isReadable && KPABase::isVideo(currentFile);

    m_crashSentinel.suspend();
    if (isReadable) {
        if (isVideo) {
            m_display = m_videoDisplay;
            m_crashSentinel.activate();
        } else
            m_display = m_imageDisplay;
    } else {
        m_display = m_textDisplay;
        m_textDisplay->setText(i18n("File not available"));
        updateInfoBox();
    }

    setCurrentWidget(m_display);
    m_infoBox->raise();

    updateContextMenuState(isVideo);

    Q_EMIT soughtTo(currentFile);

    bool ok = m_display->setImage(currentInfo(), m_forward);
    if (!ok) {
        close();
        return;
    }

    setCaptionWithDetail(QString());

    if (isVideo)
        updateCategoryConfig();

    if (m_isRunningSlideShow)
        m_slideShowTimer->start(m_slideShowPause);

    if (m_display == m_textDisplay)
        updateInfoBox();

    // Add all tagged areas
    setTaggedAreasFromImage();
}

void Viewer::ViewerWidget::setCaptionWithDetail(const QString &detail)
{
    const auto currentFile = currentFileName();
    if (currentFile.isNull())
        return;

    setWindowTitle(i18nc("@title:window %1 is the filename, %2 its detail info", "%1 %2",
                         currentFile.absolute(),
                         detail));
}

void Viewer::ViewerWidget::slotRemoveDeletedImages(const DB::FileNameList &imageList)
{
    const auto currentFile = currentFileName();
    for (const auto &filename : imageList) {
        m_list.removeAll(filename);
        m_removed.removeAll(filename);
    }
    if (m_list.isEmpty()) {
        close();
        return;
    }

    const int newIndex = m_list.indexOf(currentFile);
    if (newIndex == -1) {
        // find some sensible file to display in place of the deleted file
        if (m_current >= m_list.count()) {
            m_current = m_list.size();
            showPrev();
        } else {
            showNextN(0);
        }
    } else {
        m_current = newIndex;
        showNextN(0);
    }
}

void Viewer::ViewerWidget::contextMenuEvent(QContextMenuEvent *e)
{
    if (m_display == m_videoDisplay) {
        if (m_videoDisplay->isPaused())
            m_playPause->setText(i18nc("@action:inmenu Start video playback", "Play"));
        else
            m_playPause->setText(i18nc("@action:inmenu Pause video playback", "Pause"));

        m_stop->setEnabled(m_videoDisplay->isPlaying());
    }

    m_popup->exec(e->globalPos());
    e->setAccepted(true);
}

void Viewer::ViewerWidget::showNextN(int n)
{
    filterNone();
    if (m_display == m_videoDisplay) {
        m_videoPlayerStoppedManually = true;
        m_videoDisplay->stop();
    }

    if (m_current + n < (int)m_list.count()) {
        m_current += n;
        if (m_current >= (int)m_list.count())
            m_current = (int)m_list.count() - 1;
        m_forward = true;
        load();
    }
}

void Viewer::ViewerWidget::showNext()
{
    showNextN(1);
}

void Viewer::ViewerWidget::removeCurrent()
{
    removeOrDeleteCurrent(OnlyRemoveFromViewer);
}

void Viewer::ViewerWidget::deleteCurrent()
{
    removeOrDeleteCurrent(RemoveImageFromDatabase);
}

void Viewer::ViewerWidget::removeOrDeleteCurrent(RemoveAction action)
{
    const DB::FileName fileName = currentFileName();
    if (fileName.isNull())
        return;

    if (action == RemoveImageFromDatabase)
        m_removed.append(fileName);
    m_list.removeAll(fileName);
    if (m_list.isEmpty())
        close();
    if (m_current == m_list.count())
        showPrev();
    else
        showNextN(0);
}

void Viewer::ViewerWidget::setTagMode(TagMode tagMode)
{
    m_tagMode = tagMode;
    m_addTagAction->setEnabled(tagMode == TagMode::Annotating);
    m_copyAction->setEnabled(tagMode == TagMode::Annotating);
    m_addDescriptionAction->setEnabled(tagMode == TagMode::Annotating);

    const auto tagModeText = [&] {
        switch (tagMode) {
        case TagMode::Locked:
            return i18n("locked");
        case TagMode::Annotating:
            return i18n("annotating");
        case TagMode::Tokenizing:
            return i18n("tokenizing");
        }
        return QString();
    }();

    m_transientDisplay->display(i18n("Change display mode to %1", tagModeText));
}

void Viewer::ViewerWidget::updateContextMenuState(bool isVideo)
{
    const auto currentFile = currentFileName();
    if (currentFile.isNull())
        return;

    m_categoryImagePopup->setEnabled(!isVideo);

    m_showExifViewer->setEnabled(!isVideo);
    if (m_exifViewer)
        m_exifViewer->setImage(currentFile);

    for (QAction *videoAction : std::as_const(m_videoActions)) {
        videoAction->setVisible(isVideo);
    }

    // PENDING(blackie) This needs to be improved, so that it shows the actions only if there are that many images to jump.
    for (QList<QAction *>::const_iterator it = m_forwardActions.constBegin(); it != m_forwardActions.constEnd(); ++it)
        (*it)->setEnabled(m_current + 1 < (int)m_list.count());
    for (QList<QAction *>::const_iterator it = m_backwardActions.constBegin(); it != m_backwardActions.constEnd(); ++it)
        (*it)->setEnabled(m_current > 0);

    m_setStackHead->setEnabled(currentInfo()->isStacked());
    m_filterMenu->setEnabled(!isVideo);

    bool on = (m_list.count() > 1);
    m_startStopSlideShow->setEnabled(on);
    m_slideShowRunFaster->setEnabled(on);
    m_slideShowRunSlower->setEnabled(on);
}

namespace Viewer
{
class TemporarilyDisableCursorHandling
{
public:
    TemporarilyDisableCursorHandling(Viewer::ViewerWidget *viewer)
        : m_viewer(viewer)
    {
        viewer->m_cursorHandlerForImageDisplay->disableCursorHiding();
        viewer->m_cursorHandlerForVideoDisplay->disableCursorHiding();
    }
    ~TemporarilyDisableCursorHandling()
    {
        m_viewer->m_cursorHandlerForImageDisplay->enableCursorHiding();
        m_viewer->m_cursorHandlerForVideoDisplay->enableCursorHiding();
    }

private:
    Viewer::ViewerWidget *m_viewer;
};
}

void Viewer::ViewerWidget::showNext10()
{
    showNextN(10);
}

void Viewer::ViewerWidget::showNext100()
{
    showNextN(100);
}

void Viewer::ViewerWidget::showNext1000()
{
    showNextN(1000);
}

void Viewer::ViewerWidget::showPrevN(int n)
{
    if (m_display == m_videoDisplay)
        m_videoDisplay->stop();

    if (m_current > 0) {
        m_current -= n;
        if (m_current < 0)
            m_current = 0;
        m_forward = false;
        load();
    }
}

void Viewer::ViewerWidget::showPrev()
{
    showPrevN(1);
}

void Viewer::ViewerWidget::showPrev10()
{
    showPrevN(10);
}

void Viewer::ViewerWidget::showPrev100()
{
    showPrevN(100);
}

void Viewer::ViewerWidget::showPrev1000()
{
    showPrevN(1000);
}

void Viewer::ViewerWidget::rotate(int angle)
{
    const auto current = currentInfo();
    if (current->isNull())
        return;

    current->rotate(angle);
    m_display->rotate(current);
    invalidateThumbnail();
    MainWindow::DirtyIndicator::markDirty();
    Q_EMIT imageRotated(currentFileName());
}

void Viewer::ViewerWidget::showFirst()
{
    showPrevN(m_list.count());
}

void Viewer::ViewerWidget::showLast()
{
    showNextN(m_list.count());
}

void Viewer::ViewerWidget::closeEvent(QCloseEvent *event)
{
    if (!m_removed.isEmpty()) {
        MainWindow::DeleteDialog dialog(this);
        dialog.exec(m_removed);
    }

    m_slideShowTimer->stop();
    m_isRunningSlideShow = false;
    // give the video display time to do cleanup as long as the window handle is still valid:
    m_videoDisplay->stop();
    event->accept();
}

DB::ImageInfoPtr Viewer::ViewerWidget::currentInfo() const
{
    const auto currentFile = currentFileName();
    if (currentFile.isNull())
        return {};

    return DB::ImageDB::instance()->info(currentFile);
}

void Viewer::ViewerWidget::updatePalette()
{
    QPalette pal = palette();
    // if the scheme was set at startup from the scheme path (and not afterwards through KColorSchemeManager),
    // then KColorScheme would use the standard system scheme if we don't explicitly give a config:
    const auto schemeCfg = KSharedConfig::openConfig(Settings::SettingsData::instance()->colorScheme());
    KColorScheme::adjustBackground(pal, KColorScheme::NormalBackground, QPalette::Base, KColorScheme::Complementary, schemeCfg);
    KColorScheme::adjustForeground(pal, KColorScheme::NormalText, QPalette::Text, KColorScheme::Complementary, schemeCfg);
    setPalette(pal);
}

void Viewer::ViewerWidget::infoBoxMove()
{
    QPoint p = mapFromGlobal(QCursor::pos());
    Settings::Position oldPos = Settings::SettingsData::instance()->infoBoxPosition();
    Settings::Position pos = oldPos;
    int x = m_display->mapFromParent(p).x();
    int y = m_display->mapFromParent(p).y();
    int w = m_display->width();
    int h = m_display->height();

    if (x < w / 3) {
        if (y < h / 3)
            pos = Settings::TopLeft;
        else if (y > h * 2 / 3)
            pos = Settings::BottomLeft;
        else
            pos = Settings::Left;
    } else if (x > w * 2 / 3) {
        if (y < h / 3)
            pos = Settings::TopRight;
        else if (y > h * 2 / 3)
            pos = Settings::BottomRight;
        else
            pos = Settings::Right;
    } else {
        if (y < h / 3)
            pos = Settings::Top;
        else if (y > h * 2 / 3)
            pos = Settings::Bottom;
    }
    if (pos != oldPos) {
        Settings::SettingsData::instance()->setInfoBoxPosition(pos);
        updateInfoBox();
    }
}

void Viewer::ViewerWidget::moveInfoBox()
{
    m_infoBox->setSize();
    Settings::Position pos = Settings::SettingsData::instance()->infoBoxPosition();

    int lx = m_display->pos().x();
    int ly = m_display->pos().y();
    int lw = m_display->width();
    int lh = m_display->height();

    int bw = m_infoBox->width();
    int bh = m_infoBox->height();

    int bx, by;
    // x-coordinate
    if (pos == Settings::TopRight || pos == Settings::BottomRight || pos == Settings::Right)
        bx = lx + lw - 5 - bw;
    else if (pos == Settings::TopLeft || pos == Settings::BottomLeft || pos == Settings::Left)
        bx = lx + 5;
    else
        bx = lx + lw / 2 - bw / 2;

    // Y-coordinate
    if (pos == Settings::TopLeft || pos == Settings::TopRight || pos == Settings::Top)
        by = ly + 5;
    else if (pos == Settings::BottomLeft || pos == Settings::BottomRight || pos == Settings::Bottom)
        by = ly + lh - 5 - bh;
    else
        by = ly + lh / 2 - bh / 2;

    m_infoBox->move(bx, by);
}

void Viewer::ViewerWidget::resizeEvent(QResizeEvent *e)
{
    moveInfoBox();
    QWidget::resizeEvent(e);
}

void Viewer::ViewerWidget::updateInfoBox()
{
    if (currentInfo()) {
        QMap<int, QPair<QString, QString>> map;
        const QString text = Utilities::createInfoText(currentInfo(), &map);

        if (Settings::SettingsData::instance()->showInfoBox() && !text.isNull() && (m_type == UsageType::FullFeaturedViewer)) {
            m_infoBox->setInfo(text, map);
            m_infoBox->show();
        } else
            m_infoBox->hide();

        moveInfoBox();
    }
    m_infoBox->setSize();
}

Viewer::ViewerWidget::~ViewerWidget()
{
    inhibitScreenSaver(false);

    if (s_latest == this)
        s_latest = nullptr;
}

void Viewer::ViewerWidget::toggleFullScreen()
{
    setShowFullScreen(!m_showingFullScreen);
}

void Viewer::ViewerWidget::slotStartStopSlideShow()
{
    bool wasRunningSlideShow = m_isRunningSlideShow;
    m_isRunningSlideShow = !m_isRunningSlideShow && m_list.count() != 1;

    if (wasRunningSlideShow) {
        m_startStopSlideShow->setText(i18nc("@action:inmenu", "Run Slideshow"));
        m_slideShowTimer->stop();
        if (m_list.count() != 1)
            m_transientDisplay->display(i18nc("OSD for slideshow", "Ending Slideshow"));
        inhibitScreenSaver(false);
    } else {
        m_startStopSlideShow->setText(i18nc("@action:inmenu", "Stop Slideshow"));
        if (currentInfo()->mediaType() != DB::Video)
            m_slideShowTimer->start(m_slideShowPause);
        const auto faster = m_actions->action(QString::fromLatin1("viewer-run-faster"))->shortcut().toString();
        const auto slower = m_actions->action(QString::fromLatin1("viewer-run-slower"))->shortcut().toString();
        m_transientDisplay->display(i18nc("OSD for slideshow", "Starting Slideshow<br/>%1 makes the slideshow faster<br/>%2 makes the slideshow slower",
                                          faster, slower),
                                    1500ms);
        inhibitScreenSaver(true);
    }
}

void Viewer::ViewerWidget::slotSlideShowNextFromTimer()
{
    // Load the next images.
    QElapsedTimer timer;
    timer.start();
    if (m_display == m_imageDisplay)
        slotSlideShowNext();

    // ensure that there is a few milliseconds pause, so that an end slideshow keypress
    // can get through immediately, we don't want it to queue up behind a bunch of timer events,
    // which loaded a number of new images before the slideshow stops
    int ms = qMax(Q_INT64_C(200), m_slideShowPause - timer.elapsed());
    m_slideShowTimer->start(ms);
}

void Viewer::ViewerWidget::slotSlideShowNext()
{
    m_forward = true;
    if (m_current + 1 < (int)m_list.count())
        m_current++;
    else
        m_current = 0;

    load();
}

void Viewer::ViewerWidget::slotSlideShowFaster()
{
    changeSlideShowInterval(-500);
}

void Viewer::ViewerWidget::slotSlideShowSlower()
{
    changeSlideShowInterval(+500);
}

void Viewer::ViewerWidget::changeSlideShowInterval(int delta)
{
    if (m_list.count() == 1)
        return;

    m_slideShowPause += delta;
    m_slideShowPause = qMax(m_slideShowPause, 500);
    m_transientDisplay->display(i18nc("OSD for slideshow, num of seconds per image", "%1&nbsp;s", m_slideShowPause / 1000.0));
    if (m_slideShowTimer->isActive())
        m_slideShowTimer->start(m_slideShowPause);
}

void Viewer::ViewerWidget::editImage()
{
    // don't block this method because the ViewerWidget may already be deleted once configureImages returns
    QTimer::singleShot(0, [&]() {
        DB::ImageInfoList list;
        list.append(currentInfo());
        MainWindow::Window::configureImages(list, true);
    });
}

void Viewer::ViewerWidget::filterNone()
{
    if (m_display == m_imageDisplay) {
        m_imageDisplay->filterNone();
        m_filterMono->setChecked(false);
        m_filterBW->setChecked(false);
        m_filterContrastStretch->setChecked(false);
        m_filterHistogramEqualization->setChecked(false);
    }
}

void Viewer::ViewerWidget::filterSelected()
{
    // The filters that drop bit depth below 32 should be the last ones
    // so that filters requiring more bit depth are processed first
    if (m_display == m_imageDisplay) {
        m_imageDisplay->filterNone();
        if (m_filterBW->isChecked())
            m_imageDisplay->filterBW();
        if (m_filterContrastStretch->isChecked())
            m_imageDisplay->filterContrastStretch();
        if (m_filterHistogramEqualization->isChecked())
            m_imageDisplay->filterHistogramEqualization();
        if (m_filterMono->isChecked())
            m_imageDisplay->filterMono();
    }
}

void Viewer::ViewerWidget::filterBW()
{
    if (m_display == m_imageDisplay) {
        if (m_filterBW->isChecked())
            m_filterBW->setChecked(m_imageDisplay->filterBW());
        else
            filterSelected();
    }
}

void Viewer::ViewerWidget::filterContrastStretch()
{
    if (m_display == m_imageDisplay) {
        if (m_filterContrastStretch->isChecked())
            m_filterContrastStretch->setChecked(m_imageDisplay->filterContrastStretch());
        else
            filterSelected();
    }
}

void Viewer::ViewerWidget::filterHistogramEqualization()
{
    if (m_display == m_imageDisplay) {
        if (m_filterHistogramEqualization->isChecked())
            m_filterHistogramEqualization->setChecked(m_imageDisplay->filterHistogramEqualization());
        else
            filterSelected();
    }
}

void Viewer::ViewerWidget::filterMono()
{
    if (m_display == m_imageDisplay) {
        if (m_filterMono->isChecked())
            m_filterMono->setChecked(m_imageDisplay->filterMono());
        else
            filterSelected();
    }
}

void Viewer::ViewerWidget::slotSetStackHead()
{
    const auto currentFile = currentFileName();
    if (currentFile.isNull())
        return;

    MainWindow::Window::theMainWindow()->setStackHead(currentFile);
}

bool Viewer::ViewerWidget::showingFullScreen() const
{
    return m_showingFullScreen;
}

void Viewer::ViewerWidget::setShowFullScreen(bool on)
{
    if (on) {
        setWindowState(windowState() | Qt::WindowFullScreen); // set
        moveInfoBox();
    } else {
        // We need to size the image when going out of full screen, in case we started directly in full screen
        //
        setWindowState(windowState() & ~Qt::WindowFullScreen); // reset
        resize(Settings::SettingsData::instance()->viewerSize());
    }
    m_showingFullScreen = on;
}

void Viewer::ViewerWidget::updateCategoryConfig()
{
    if (!CategoryImageConfig::instance()->isVisible())
        return;

    CategoryImageConfig::instance()->setCurrentImage(m_imageDisplay->currentViewAsThumbnail(), currentInfo());
}

void Viewer::ViewerWidget::populateExternalPopup()
{
    m_externalPopup->populate(currentInfo(), m_list);
}

void Viewer::ViewerWidget::populateCategoryImagePopup()
{
    const auto currentFile = currentFileName();
    if (currentFile.isNull())
        return;

    m_categoryImagePopup->populate(m_imageDisplay->currentViewAsThumbnail(), currentFile);
}

void Viewer::ViewerWidget::show(bool slideShow)
{
    QSize size;
    bool fullScreen;
    if (slideShow) {
        fullScreen = Settings::SettingsData::instance()->launchSlideShowFullScreen();
        size = Settings::SettingsData::instance()->slideShowSize();
    } else {
        fullScreen = Settings::SettingsData::instance()->launchViewerFullScreen();
        size = Settings::SettingsData::instance()->viewerSize();
    }

    if (fullScreen)
        setShowFullScreen(true);
    else
        resize(size);

    QWidget::show();
    if (slideShow != m_isRunningSlideShow) {
        // The info dialog will show up at the wrong place if we call this function directly
        // don't ask me why -  4 Sep. 2004 15:13 -- Jesper K. Pedersen
        QTimer::singleShot(0, this, &ViewerWidget::slotStartStopSlideShow);
    }
}

KActionCollection *Viewer::ViewerWidget::actions()
{
    return m_actions;
}

void Viewer::ViewerWidget::keyPressEvent(QKeyEvent *event)
{
    const bool readOnly = m_type != UsageType::FullFeaturedViewer;
    if (readOnly) {
        event->ignore();
        return;
    }

    bool dirty = false;
    // Rating of the image
    if (event->modifiers() == 0 && event->key() >= Qt::Key_0 && event->key() <= Qt::Key_5) {
        const auto rating = event->key() - Qt::Key_0;
        currentInfo()->setRating(rating * 2);
        dirty = true;
    } else if (m_tagMode == TagMode::Locked) {
        return;
    } else if (m_tagMode == TagMode::Tokenizing) {
        if (event->key() < Qt::Key_A || event->key() > Qt::Key_Z)
            return;

        auto category = DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory)->name();
        toggleTag(category, event->text());
    } else {
        TemporarilyDisableCursorHandling dummy(this);
        dirty = m_annotationHandler->handle(event);
    }

    updateInfoBox();
    if (dirty)
        MainWindow::DirtyIndicator::markDirty();
}

void Viewer::ViewerWidget::videoStopped()
{
    if (!m_videoPlayerStoppedManually && m_isRunningSlideShow)
        slotSlideShowNext();
    m_videoPlayerStoppedManually = false;
}

void Viewer::ViewerWidget::wheelEvent(QWheelEvent *event)
{
    const auto angleDelta = event->angleDelta();
    const bool isHorizontal = (qAbs(angleDelta.x()) > qAbs(angleDelta.y()));
    if ((!isHorizontal && angleDelta.y() < 0) || (isHorizontal && angleDelta.x() < 0)) {
        showNext();
    } else {
        showPrev();
    }
}

void Viewer::ViewerWidget::showExifViewer()
{
    const auto currentFile = currentFileName();
    if (currentFile.isNull())
        return;

    m_exifViewer = new Exif::InfoDialog(currentFile, this);
    m_exifViewer->show();
}

void Viewer::ViewerWidget::zoomIn()
{
    m_display->zoomIn();
}

void Viewer::ViewerWidget::zoomOut()
{
    m_display->zoomOut();
}

void Viewer::ViewerWidget::zoomFull()
{
    m_display->zoomFull();
}

void Viewer::ViewerWidget::zoomPixelForPixel()
{
    m_display->zoomPixelForPixel();
}

void Viewer::ViewerWidget::makeThumbnailImage()
{
    VideoShooter::go(currentInfo(), this);
}

void Viewer::ViewerWidget::addTag()
{
    TemporarilyDisableCursorHandling dummy(this);
    const bool dirty = m_annotationHandler->askForTagAndInsert();
    if (dirty)
        MainWindow::DirtyIndicator::markDirty();
}

void Viewer::ViewerWidget::editDescription()
{
    TemporarilyDisableCursorHandling dummy(this);
    const auto description = currentInfo()->description();
    bool ok;
    auto newDescription = QInputDialog::getMultiLineText(this, i18nc("@title", "Edit Image Description"), i18nc("@label:textbox", "Image Description"), description, &ok);
    if (ok && description != newDescription) {
        currentInfo()->setDescription(newDescription);
        MainWindow::DirtyIndicator::markDirty();
    }
}

void Viewer::ViewerWidget::showAnnotationHelp()
{
    QDesktopServices::openUrl(QUrl(QLatin1String("help:/kphotoalbum/chp-viewer.html#annotating-from-the-viewer")));
}

void Viewer::ViewerWidget::createVideoMenu()
{
    QMenu *menu = new QMenu(m_popup);
    menu->setTitle(i18nc("@title:inmenu", "Seek"));

    m_videoActions.append(m_popup->addMenu(menu));

    int count = 0;
    auto add = [&](const QString &title, const char *name, int value, const QKeySequence &key) {
        if (count++ == 5) {
            QAction *sep = new QAction(menu);
            sep->setSeparator(true);
            menu->addAction(sep);
        }

        QAction *seek = m_actions->addAction(QString::fromLatin1(name));
        seek->setText(title);
        seek->setShortcut(key);
        m_actions->setShortcutsConfigurable(seek, false);
        connect(seek, &QAction::triggered, m_videoDisplay, [this, value] {
            m_videoDisplay->relativeSeek(value);
        });
        menu->addAction(seek);
    };

    add(i18nc("@action:inmenu", "10 minutes backward"), "seek-10-minute", -600000, QKeySequence(QString::fromLatin1("Ctrl+Left")));
    add(i18nc("@action:inmenu", "1 minute backward"), "seek-1-minute", -60000, QKeySequence(QString::fromLatin1("Shift+Left")));
    add(i18nc("@action:inmenu", "10 seconds backward"), "seek-10-second", -10000, QKeySequence(QString::fromLatin1("Left")));
    add(i18nc("@action:inmenu", "1 seconds backward"), "seek-1-second", -1000, QKeySequence(QString::fromLatin1("Up")));
    add(i18nc("@action:inmenu", "100 milliseconds backward"), "seek-100-millisecond", -100, QKeySequence(QString::fromLatin1("Shift+Up")));
    add(i18nc("@action:inmenu", "100 milliseconds forward"), "seek+100-millisecond", 100, QKeySequence(QString::fromLatin1("Shift+Down")));
    add(i18nc("@action:inmenu", "1 seconds forward"), "seek+1-second", 1000, QKeySequence(QString::fromLatin1("Down")));
    add(i18nc("@action:inmenu", "10 seconds forward"), "seek+10-second", 10000, QKeySequence(QString::fromLatin1("Right")));
    add(i18nc("@action:inmenu", "1 minute forward"), "seek+1-minute", 60000, QKeySequence(QString::fromLatin1("Shift+Right")));
    add(i18nc("@action:inmenu", "10 minutes forward"), "seek+10-minute", 600000, QKeySequence(QString::fromLatin1("Ctrl+Right")));

    QAction *sep = new QAction(m_popup);
    sep->setSeparator(true);
    m_popup->addAction(sep);
    m_videoActions.append(sep);

    m_stop = m_actions->addAction(QString::fromLatin1("viewer-video-stop"), m_videoDisplay, &VideoDisplay::stop);
    m_stop->setText(i18nc("@action:inmenu Stop video playback", "Stop"));
    m_popup->addAction(m_stop);
    m_videoActions.append(m_stop);

    m_playPause = m_actions->addAction(QString::fromLatin1("viewer-video-pause"), m_videoDisplay, &VideoDisplay::playPause);
    // text set in contextMenuEvent()
    m_playPause->setShortcut(Qt::Key_P);
    m_actions->setShortcutsConfigurable(m_playPause, false);
    m_popup->addAction(m_playPause);
    m_videoActions.append(m_playPause);

    m_makeThumbnailImage = m_actions->addAction(QString::fromLatin1("make-thumbnail-image"), this, &ViewerWidget::makeThumbnailImage);
    m_makeThumbnailImage->setText(i18nc("@action:inmenu", "Use current frame in thumbnail view"));
    m_makeThumbnailImage->setVisible(m_type == UsageType::FullFeaturedViewer);
    m_popup->addAction(m_makeThumbnailImage);
    m_videoActions.append(m_makeThumbnailImage);

    QAction *restart = m_actions->addAction(QString::fromLatin1("viewer-video-restart"), m_videoDisplay, &VideoDisplay::restart);
    m_actions->setDefaultShortcut(restart, Qt::Key_Home);
    restart->setText(i18nc("@action:inmenu Restart video playback.", "Restart"));
    m_popup->addAction(restart);
    m_videoActions.append(restart);
}

void Viewer::ViewerWidget::createCategoryImageMenu()
{
    m_categoryImagePopup = new MainWindow::CategoryImagePopup(m_popup);
    const bool showFullFeatures = m_type == UsageType::FullFeaturedViewer;
    m_popup->addMenu(m_categoryImagePopup)->setVisible(showFullFeatures);
    connect(m_categoryImagePopup, &MainWindow::CategoryImagePopup::aboutToShow, this, &ViewerWidget::populateCategoryImagePopup);
}

void Viewer::ViewerWidget::createFilterMenu()
{
    m_filterMenu = new QMenu(m_popup);
    m_filterMenu->setTitle(i18nc("@title:inmenu", "Filters"));

    m_filterNone = m_actions->addAction(QString::fromLatin1("filter-empty"), this, &ViewerWidget::filterNone);
    m_filterNone->setText(i18nc("@action:inmenu", "Remove All Filters"));
    m_filterMenu->addAction(m_filterNone);

    m_filterBW = m_actions->addAction(QString::fromLatin1("filter-bw"), this, &ViewerWidget::filterBW);
    m_filterBW->setText(i18nc("@action:inmenu", "Apply Grayscale Filter"));
    m_filterBW->setCheckable(true);
    m_filterMenu->addAction(m_filterBW);

    m_filterContrastStretch = m_actions->addAction(QString::fromLatin1("filter-cs"), this, &ViewerWidget::filterContrastStretch);
    m_filterContrastStretch->setText(i18nc("@action:inmenu", "Apply Contrast Stretching Filter"));
    m_filterContrastStretch->setCheckable(true);
    m_filterMenu->addAction(m_filterContrastStretch);

    m_filterHistogramEqualization = m_actions->addAction(QString::fromLatin1("filter-he"), this, &ViewerWidget::filterHistogramEqualization);
    m_filterHistogramEqualization->setText(i18nc("@action:inmenu", "Apply Histogram Equalization Filter"));
    m_filterHistogramEqualization->setCheckable(true);
    m_filterMenu->addAction(m_filterHistogramEqualization);

    m_filterMono = m_actions->addAction(QString::fromLatin1("filter-mono"), this, &ViewerWidget::filterMono);
    m_filterMono->setText(i18nc("@action:inmenu", "Apply Monochrome Filter"));
    m_filterMono->setCheckable(true);
    m_filterMenu->addAction(m_filterMono);

    const bool showFullFeatures = m_type == UsageType::FullFeaturedViewer;
    // hide entries of hidden menus so that they can't be triggered via shortcut:
    for (auto &action : m_filterMenu->actions())
        action->setVisible(showFullFeatures);
    m_popup->addMenu(m_filterMenu)->setVisible(showFullFeatures);
}

void Viewer::ViewerWidget::test()
{
#ifdef TESTING
    QTimeLine *timeline = new QTimeLine;
    timeline->setStartFrame(_infoBox->y());
    timeline->setEndFrame(height());
    connect(timeline, &QTimeLine::frameChanged, this, &ViewerWidget::moveInfoBox);
    timeline->start();
#endif // TESTING
}

void Viewer::ViewerWidget::moveInfoBox(int y)
{
    m_infoBox->move(m_infoBox->x(), y);
}

namespace Viewer
{
static VideoDisplay *instantiateVideoDisplay(QWidget *parent, KPABase::CrashSentinel &sentinel)
{
    auto backend = Settings::SettingsData::instance()->videoBackend();
    if (backend == Settings::VideoBackend::NotConfigured) {
        // just select a backend for the user if they didn't choose one
        backend = Settings::preferredVideoBackend(backend);
    }
    if (sentinel.hasCrashInfo()) {
        // KPA crashed during video playback - time to select a different backend based on crash data:
        const auto badBackends = sentinel.crashHistory();
        const auto backendEnum = QMetaEnum::fromType<Settings::VideoBackend>();
        Settings::VideoBackends exclusions;
        for (const auto &badBackend : badBackends) {
            bool ok = false;
            const auto be = static_cast<Settings::VideoBackend>(backendEnum.keyToValue(badBackend.constData(), &ok));
            if (ok) {
                exclusions |= be;
            } else {
                qCWarning(ViewerLog) << "Could not parse crash data:" << badBackend << "is an unknown video backend value! Ignoring...";
            }
        }
        auto preferredBackend = Settings::preferredVideoBackend(backend, exclusions);
        if (preferredBackend != backend) {
            qCWarning(ViewerLog) << "A crash was registered during usage of the " << backend << "video backend - preferred new backend:" << preferredBackend;
            const bool foundViableBackend = (preferredBackend != Settings::VideoBackend::NotConfigured);
            if (foundViableBackend) {
                const auto message = i18n(
                    "<p>It seems that KPhotoAlbum previously crashed during video playback. "
                    "On some platforms, this is a common problem with some video players.</p>"
                    "<p>Press <i>Continue</i> to let KPhotoAlbum try a different backend...</p>");
                const auto messageDetails = i18n(
                    "<p>Video backend that was interrupted: <tt>%1</tt></p>"
                    "<p>Video backend that will be used instead: <tt>%2</tt></p>",
                    Settings::localizedEnumName(backend), Settings::localizedEnumName(preferredBackend));
                const auto choice = KMessageBox::warningContinueCancelDetailed(parent, message, QString(), KStandardGuiItem::cont(), KStandardGuiItem::cancel(), QString(), KMessageBox::Notify, messageDetails);
                if (choice == KMessageBox::Continue) {
                    Settings::SettingsData::instance()->setVideoBackend(preferredBackend);
                    backend = preferredBackend;
                }
            } else {
                // if no viable backend was found, that means that all available backends crashed at some point
                // i.e. there's no point in bugging the user again - just disable the crash detection completely
                sentinel.disablePermanently();
                const auto message = i18n(
                    "<p>KPhotoAlbum has tried out all available video backends, but every one crashed at some point.</p>"
                    "<p>Crash detection is now turned off.</p>");
                KMessageBox::error(parent, message);
            }
        }
    }
    bool showSelectorDialog = backend == Settings::VideoBackend::NotConfigured;

    if (showSelectorDialog) {
        // no viable backend found yet -> we need the user to choose
        Settings::VideoPlayerSelectorDialog dialog;
        dialog.exec();
        Settings::SettingsData::instance()->setVideoBackend(dialog.backend());
        backend = Settings::SettingsData::instance()->videoBackend();
        sentinel.clearCrashHistory();
    }

    switch (backend) {
    case Settings::VideoBackend::VLC:
#if LIBVLC_FOUND
        return new VLCDisplay(parent);
#else
        qCWarning(ViewerLog) << "Video backend VLC not available. Selecting first available backend...";
#endif
        break;
    case Settings::VideoBackend::QtAV:
#if QtAV_FOUND
        return new QtAVDisplay(parent);
#else
        qCWarning(ViewerLog) << "Video backend QtAV not available. Selecting first available backend...";
#endif
        break;
    case Settings::VideoBackend::Phonon:
#if Phonon4Qt5_FOUND
        return new PhononDisplay(parent);
#else
        qCWarning(ViewerLog) << "Video backend Phonon not available. Selecting first available backend...";
#endif
        break;
    case Settings::VideoBackend::NotConfigured:
        qCCritical(ViewerLog) << "No viable video backend!";
    }

    static_assert(LIBVLC_FOUND || QtAV_FOUND || Phonon4Qt5_FOUND, "A video backend must be provided. The build system should bail out if none is available.");
    Q_UNREACHABLE();
    return nullptr;
}
}

void Viewer::ViewerWidget::createVideoViewer()
{

    m_videoDisplay = instantiateVideoDisplay(this, m_crashSentinel);
    const auto backendEnum = QMetaEnum::fromType<Settings::VideoBackend>();
    const auto backendName = backendEnum.valueToKey(static_cast<int>(Settings::SettingsData::instance()->videoBackend()));
    m_crashSentinel.setCrashInfo(backendName);

    addWidget(m_videoDisplay);
    connect(m_videoDisplay, &VideoDisplay::stopped, this, &ViewerWidget::videoStopped);
    m_cursorHandlerForVideoDisplay = new CursorVisibilityHandler(m_videoDisplay);
}

void Viewer::ViewerWidget::createAnnotationMenu()
{
    auto menu = new QMenu(i18n("Annotate"));

    auto addAction = [&](const char *name, const QString &title, auto slot, auto shortCut) {
        QAction *action = m_actions->addAction(QString::fromLatin1(name), this, slot);
        action->setText(title);
        m_actions->setDefaultShortcut(action, shortCut);
        menu->addAction(action);
        return action;
    };

    auto toggleGroup = new QActionGroup(this);
    auto addTagAction = [&](const char *name, const QString &title, TagMode mode, auto shortCut) {
        auto action = addAction(
            name, title, [this, mode] { setTagMode(mode); }, shortCut);
        action->setCheckable(true);
        toggleGroup->addAction(action);
        return action;
    };

    addAction("viewer-show-keybindings", i18nc("@action:inmenu", "Help"), &ViewerWidget::showAnnotationHelp, QKeySequence(Qt::CTRL, Qt::Key_Question));

    addAction("viewer-edit-image-properties", i18nc("@action:inmenu", "Annotation Dialog"), &ViewerWidget::editImage, QKeySequence(Qt::CTRL, Qt::Key_1));
    m_addTagAction = addAction("viewer-add-tag", i18nc("@action:inmenu", "Add tag"), &ViewerWidget::addTag, i18nc("short cut for add tag", "CTRL+a"));
    m_addTagAction->setEnabled(false);

    m_copyAction = addAction("viewer-copy-tag-from-previous-image", i18nc("@action:inmenu", "Copy Data from Previous Image"), &ViewerWidget::copyTagsFromPreviousImage,
                             i18nc("Shortcut for copy annotations from previous image", "CTRL+c"));
    m_copyAction->setEnabled(false);

    m_addDescriptionAction = addAction("viewer-edit-description", i18nc("@action:inmenu", "Edit Description"), &ViewerWidget::editDescription,
                                       i18nc("Shortcut for add description to image", "CTRL+d"));
    m_addDescriptionAction->setEnabled(false);

    menu->addSection(i18n("Annotation Mode"));
    auto action = addTagAction(
        "viewer-tagmode-locked", i18nc("@action:inmenu", "Locked"), TagMode::Locked,
        i18nc("Shortcut for turning of annotations in the viewer", "CTRL+l"));
    action->setChecked(true);

    addTagAction(
        "viewer-tagmode-annotating", i18nc("@action:inmenu", "Assign Tags"), TagMode::Annotating,
        i18nc("Shortcut for turning annotations mode to annotating", "F2"));

    addTagAction(
        "viewer-tagmode-tokenizing", i18nc("@action:inmenu", "Assign Tokens"), TagMode::Tokenizing,
        i18nc("Shortcut for turning annotations mode to tokenizing", "CTRL+t"));

    const bool showFullFeatures = m_type == UsageType::FullFeaturedViewer;
    // hide entries of hidden menus so that they can't be triggered via shortcut:
    for (auto &action : menu->actions())
        action->setVisible(showFullFeatures);
    m_popup->addMenu(menu)->setVisible(showFullFeatures);
}

void Viewer::ViewerWidget::stopPlayback()
{
    m_videoDisplay->stop();
    m_crashSentinel.suspend();
}

void Viewer::ViewerWidget::invalidateThumbnail() const
{
    const auto currentFile = currentFileName();
    if (currentFile.isNull())
        return;

    MainWindow::Window::theMainWindow()->thumbnailCache()->removeThumbnail(currentFile);
}

void Viewer::ViewerWidget::setTaggedAreasFromImage()
{
    // Clean all areas we probably already have
    const auto allAreas = findChildren<TaggedArea *>();
    for (TaggedArea *area : allAreas) {
        area->deleteLater();
    }

    DB::TaggedAreas taggedAreas = currentInfo()->taggedAreas();
    addTaggedAreas(taggedAreas, AreaType::Standard);
}

void Viewer::ViewerWidget::addAdditionalTaggedAreas(DB::TaggedAreas taggedAreas)
{
    addTaggedAreas(taggedAreas, AreaType::Highlighted);
}

void Viewer::ViewerWidget::addTaggedAreas(DB::TaggedAreas taggedAreas, AreaType type)
{
    DB::TaggedAreasIterator areasInCategory(taggedAreas);
    QString category;
    QString tag;

    while (areasInCategory.hasNext()) {
        areasInCategory.next();
        category = areasInCategory.key();

        DB::PositionTagsIterator areaData(areasInCategory.value());
        while (areaData.hasNext()) {
            areaData.next();
            tag = areaData.key();

            // Add a new frame for the area
            TaggedArea *newArea = new TaggedArea(this);
            newArea->setTagInfo(category, category, tag);
            newArea->setActualGeometry(areaData.value());
            newArea->setHighlighted(type == AreaType::Highlighted);
            newArea->show();

            connect(m_infoBox, &InfoBox::tagHovered, newArea, &TaggedArea::checkIsSelected);
            connect(m_infoBox, &InfoBox::noTagHovered, newArea, &TaggedArea::deselect);
        }
    }

    // Be sure to display the areas, as viewGeometryChanged is not always emitted on load

    QSize imageSize = currentInfo()->size();
    QSize windowSize = this->size();

    // On load, the image is never zoomed, so it's a bit easier ;-)
    double scaleWidth = double(imageSize.width()) / windowSize.width();
    double scaleHeight = double(imageSize.height()) / windowSize.height();
    int offsetTop = 0;
    int offsetLeft = 0;
    if (scaleWidth > scaleHeight) {
        offsetTop = (windowSize.height() - imageSize.height() / scaleWidth);
    } else {
        offsetLeft = (windowSize.width() - imageSize.width() / scaleHeight);
    }

    remapAreas(
        QSize(windowSize.width() - offsetLeft, windowSize.height() - offsetTop),
        QRect(QPoint(0, 0), QPoint(imageSize.width(), imageSize.height())),
        1);
}

void Viewer::ViewerWidget::remapAreas(QSize viewSize, QRect zoomWindow, double sizeRatio)
{
    QSize currentWindowSize = this->size();
    int outerOffsetLeft = (currentWindowSize.width() - viewSize.width()) / 2;
    int outerOffsetTop = (currentWindowSize.height() - viewSize.height()) / 2;

    if (sizeRatio != 1) {
        zoomWindow = QRect(
            QPoint(
                double(zoomWindow.left()) * sizeRatio,
                double(zoomWindow.top()) * sizeRatio),
            QPoint(
                double(zoomWindow.left() + zoomWindow.width()) * sizeRatio,
                double(zoomWindow.top() + zoomWindow.height()) * sizeRatio));
    }

    double scaleHeight = double(viewSize.height()) / zoomWindow.height();
    double scaleWidth = double(viewSize.width()) / zoomWindow.width();

    int innerOffsetLeft = -zoomWindow.left() * scaleWidth;
    int innerOffsetTop = -zoomWindow.top() * scaleHeight;

    const auto areas = findChildren<TaggedArea *>();
    for (TaggedArea *area : areas) {
        const QRect actualGeometry = area->actualGeometry();
        QRect screenGeometry;

        screenGeometry.setWidth(actualGeometry.width() * scaleWidth);
        screenGeometry.setHeight(actualGeometry.height() * scaleHeight);
        screenGeometry.moveTo(
            actualGeometry.left() * scaleWidth + outerOffsetLeft + innerOffsetLeft,
            actualGeometry.top() * scaleHeight + outerOffsetTop + innerOffsetTop);

        area->setGeometry(screenGeometry);
    }
}

void Viewer::ViewerWidget::setCopyLinkEngine(MainWindow::CopyLinkEngine *copyLinkEngine)
{
    m_copyLinkEngine = copyLinkEngine;
}

void Viewer::ViewerWidget::triggerCopyLinkAction(MainWindow::CopyLinkEngine::Action action)
{
    const auto currentFile = currentFileName();
    if (currentFile.isNull())
        return;

    if (!m_copyLinkEngine) {
        qCWarning(ViewerLog) << "ViewerWidget::triggerCopyLinkAction called without CopyLinkEngine. This is a bug!";
        return;
    }
    const auto selectedFiles = QList<QUrl> { QUrl::fromLocalFile(currentFile.absolute()) };
    m_copyLinkEngine->selectTarget(this, selectedFiles, action);
}

void Viewer::ViewerWidget::toggleTag(const QString &category, const QString &value)
{
    QString tag = value;
    if (category == DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory)->name())
        tag = value.toUpper();

    const bool tagIsSet = !currentInfo()->hasCategoryInfo(category, tag);
    if (tagIsSet)
        currentInfo()->addCategoryInfo(category, tag);
    else
        currentInfo()->removeCategoryInfo(category, tag);

    // Assume we've now annotated this image - this is to avoid removing the untagged item all the time.
    currentInfo()->removeCategoryInfo(Settings::SettingsData::instance()->untaggedCategory(), Settings::SettingsData::instance()->untaggedTag());
    updateInfoBox();

    if (category == DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory)->name())
        tag = i18n("Token %1", tag);
    m_transientDisplay->display(tagIsSet ? tag : QLatin1String("<s>%1</s>").arg(tag), 500ms, TransientDisplay::NoFadeOut);
}

void Viewer::ViewerWidget::copyTagsFromPreviousImage()
{
    // Search for the previous image - that is the first one not deleted
    int index = m_current - 1;
    while (index >= 0) {
        const auto fileName = m_list.at(index);
        if (!m_removed.contains(fileName))
            break;
        --index;
    }
    if (index == -1)
        return; // Nothing found

    const auto prevImage = DB::ImageDB::instance()->info(m_list[index]);
    currentInfo()->merge(*prevImage);

    updateInfoBox();
    MainWindow::DirtyIndicator::markDirty();
}

#include "moc_ViewerWidget.cpp"

// vi:expandtab:tabstop=4 shiftwidth=4:
