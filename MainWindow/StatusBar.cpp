/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#include "StatusBar.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QProgressBar>
#include <QSlider>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <KIconLoader>
#include <KLocalizedString>

#include <BackgroundTaskManager/StatusIndicator.h>
#include <DB/ImageDB.h>
#include <RemoteControl/ConnectionIndicator.h>
#include <Settings/SettingsData.h>
#include <ThumbnailView/ThumbnailFacade.h>

#include "DirtyIndicator.h"
#include "ImageCounter.h"

MainWindow::StatusBar::StatusBar()
    : QStatusBar()
{
    QPalette pal = palette();
    pal.setBrush(QPalette::Base, QApplication::palette().color(QPalette::Background));
    pal.setBrush(QPalette::Background, QApplication::palette().color(QPalette::Background));
    setPalette(pal);

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
    connect(DB::ImageDB::instance(), SIGNAL(dirty()), mp_dirtyIndicator, SLOT(markDirtySlot()));

    auto *remoteIndicator = new RemoteControl::ConnectionIndicator(indicators);
    indicatorsHBoxLayout->addWidget(remoteIndicator);

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
    connect(DB::ImageDB::instance(), SIGNAL(totalChanged(uint)), total, SLOT(setTotal(uint)));

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
    static QPixmap *lockedPix = new QPixmap(SmallIcon(QString::fromLatin1("object-locked")));
    m_lockedIndicator->setFixedWidth(lockedPix->width());

    if (locked)
        m_lockedIndicator->setPixmap(*lockedPix);
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
    static QTime time;
    if (time.isNull() || time.elapsed() > 200) {
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
