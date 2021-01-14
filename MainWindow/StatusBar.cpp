/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "StatusBar.h"

#include "DirtyIndicator.h"
#include "ImageCounter.h"

#include <BackgroundTaskManager/StatusIndicator.h>
#include <DB/ImageDB.h>
#ifdef KPA_ENABLE_REMOTECONTROL
#include <RemoteControl/ConnectionIndicator.h>
#endif
#include <ThumbnailView/ThumbnailFacade.h>
#include <kpabase/SettingsData.h>

#include <KIconLoader>
#include <KLocalizedString>
#include <QApplication>
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QProgressBar>
#include <QSlider>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

MainWindow::StatusBar::StatusBar()
    : QStatusBar()
{
    setupGUI();
    m_pendingShowTimer = new QTimer(this);
    m_pendingShowTimer->setSingleShot(true);
    connect(m_pendingShowTimer, &QTimer::timeout, this, &StatusBar::showStatusBar);
}

void MainWindow::StatusBar::setupGUI()
{
    setContentsMargins(7, 2, 7, 2);

    QWidget *indicators = new QWidget(this);
    QHBoxLayout *indicatorsHBoxLayout = new QHBoxLayout(indicators);
    indicatorsHBoxLayout->setMargin(0);
    indicatorsHBoxLayout->setSpacing(10);
    mp_dirtyIndicator = new DirtyIndicator(indicators);
    indicatorsHBoxLayout->addWidget(mp_dirtyIndicator);
    connect(DB::ImageDB::instance(), &DB::ImageDB::dirty, mp_dirtyIndicator, &DirtyIndicator::markDirtySlot);

#ifdef KPA_ENABLE_REMOTECONTROL
    auto *remoteIndicator = new RemoteControl::ConnectionIndicator(indicators);
    indicatorsHBoxLayout->addWidget(remoteIndicator);
#endif

    auto *jobIndicator = new BackgroundTaskManager::StatusIndicator(indicators);
    indicatorsHBoxLayout->addWidget(jobIndicator);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setMinimumWidth(400);
    addPermanentWidget(m_progressBar, 0);

    m_cancel = new QToolButton(this);
    m_cancel->setIcon(QIcon::fromTheme(QString::fromLatin1("dialog-close")));
    m_cancel->setShortcut(Qt::Key_Escape);
    addPermanentWidget(m_cancel, 0);
    connect(m_cancel, &QToolButton::clicked, this, &StatusBar::cancelRequest);
    connect(m_cancel, &QToolButton::clicked, this, &StatusBar::hideStatusBar);

    m_lockedIndicator = new QLabel(indicators);
    indicatorsHBoxLayout->addWidget(m_lockedIndicator);

    addPermanentWidget(indicators, 0);

    mp_partial = new ImageCounter(this);
    addPermanentWidget(mp_partial, 0);

    mp_selected = new ImageCounter(this);
    addPermanentWidget(mp_selected, 0);

    ImageCounter *total = new ImageCounter(this);
    addPermanentWidget(total, 0);
    total->setTotal(DB::ImageDB::instance()->totalCount());
    connect(DB::ImageDB::instance(), &DB::ImageDB::totalChanged, total, &ImageCounter::setTotal);

    mp_pathIndicator = new BreadcrumbViewer;
    addWidget(mp_pathIndicator, 1);

    setProgressBarVisible(false);

    m_thumbnailSizeSlider = ThumbnailView::ThumbnailFacade::instance()->createResizeSlider();
    addPermanentWidget(m_thumbnailSizeSlider, 0);
    // prevent stretching:
    m_thumbnailSizeSlider->setMaximumSize(m_thumbnailSizeSlider->size());
    m_thumbnailSizeSlider->setMinimumSize(m_thumbnailSizeSlider->size());
    m_thumbnailSizeSlider->hide();

    m_thumbnailSettings = new QToolButton;
    m_thumbnailSettings->setIcon(QIcon::fromTheme(QString::fromUtf8("settings-configure")));
    m_thumbnailSettings->setToolTip(i18n("Thumbnail settings..."));
    addPermanentWidget(m_thumbnailSettings, 0);
    m_thumbnailSettings->hide();
    connect(m_thumbnailSettings, &QToolButton::clicked, this, &StatusBar::thumbnailSettingsRequested);
}

void MainWindow::StatusBar::setLocked(bool locked)
{
    static QPixmap lockedPix = QIcon::fromTheme(QString::fromLatin1("object-locked"))
                                   .pixmap(KIconLoader::StdSizes::SizeSmall);
    m_lockedIndicator->setFixedWidth(lockedPix.width());

    if (locked)
        m_lockedIndicator->setPixmap(lockedPix);
    else
        m_lockedIndicator->setPixmap(QPixmap());
}

void MainWindow::StatusBar::startProgress(const QString &text, int total)
{
    m_progressBar->setFormat(text + QString::fromLatin1(": %p%"));
    m_progressBar->setMaximum(total);
    m_progressBar->setValue(0);
    m_pendingShowTimer->start(1000); // To avoid flicker we will only show the statusbar after 1 second.
}

void MainWindow::StatusBar::setProgress(int progress)
{
    if (progress == m_progressBar->maximum())
        hideStatusBar();

    // If progress comes in to fast, then the UI will freeze from all time spent on updating the progressbar.
    static QElapsedTimer time;
    if (!time.isValid() || time.elapsed() > 200) {
        m_progressBar->setValue(progress);
        time.restart();
    }
}

void MainWindow::StatusBar::setProgressBarVisible(bool show)
{
    m_progressBar->setVisible(show);
    m_cancel->setVisible(show);
}

void MainWindow::StatusBar::showThumbnailSlider()
{
    m_thumbnailSizeSlider->setVisible(true);
    m_thumbnailSettings->show();
}

void MainWindow::StatusBar::hideThumbnailSlider()
{
    m_thumbnailSizeSlider->setVisible(false);
    m_thumbnailSettings->hide();
}

void MainWindow::StatusBar::enterEvent(QEvent *)
{
    // make sure that breadcrumbs are not obscured by messages
    clearMessage();
}

void MainWindow::StatusBar::hideStatusBar()
{
    setProgressBarVisible(false);
    m_pendingShowTimer->stop();
}

void MainWindow::StatusBar::showStatusBar()
{
    setProgressBarVisible(true);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
