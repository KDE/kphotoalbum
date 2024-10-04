// SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2020-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ExternalPopup.h"

#include "Logging.h"
#include "RunDialog.h"
#include "Window.h"

#include <DB/ImageInfo.h>
#include <kpabase/FileNameList.h>
#include <kpabase/SettingsData.h>

#include <KApplicationTrader>
#include <KFileItem>
#include <KIO/ApplicationLauncherJob>
#include <KIO/JobUiDelegate>
#include <KIO/JobUiDelegateFactory>
#include <KLocalizedString>
#include <KService>
#include <KShell>
#include <QFile>
#include <QIcon>
#include <QLabel>
#include <QMimeDatabase>
#include <QPixmap>
#include <QRegularExpression>
#include <QStringList>
#include <QUrl>
#include <kio_version.h>
#include <kservice_version.h>

#include <utility>

void MainWindow::ExternalPopup::populate(DB::ImageInfoPtr current, const DB::FileNameList &imageList)
{
    m_list = imageList;
    m_currentInfo = current;
    clear();
    QAction *action;

    for (PopupAction which : { PopupAction::OpenCurrent, PopupAction::OpenAllSelected, PopupAction::CopyAndOpenAllSelected }) {
        if (which == PopupAction::OpenCurrent && !current)
            continue;

        const bool multiple = (m_list.count() > 1);
        const bool enabled = (which != PopupAction::OpenAllSelected && m_currentInfo) || (which == PopupAction::OpenAllSelected && multiple);

        // Submenu
        QString menuLabel;
        switch (which) {
        case PopupAction::OpenCurrent:
            menuLabel = i18n("Current Item");
            break;
        case PopupAction::OpenAllSelected:
            menuLabel = i18n("All Selected Items");
            break;
        case PopupAction::CopyAndOpenAllSelected:
            menuLabel = i18n("Copy and Open");
        }
        Q_ASSERT(!menuLabel.isNull());

        QMenu *submenu = addMenu(menuLabel);
        submenu->setEnabled(enabled);

        // Fetch set of offers
        KService::List offers;
        if (which == PopupAction::OpenCurrent)
            offers = appInfos(DB::FileNameList() << current->fileName());
        else
            offers = appInfos(imageList);

        for (KService::Ptr offer : std::as_const(offers)) {
            action = submenu->addAction(offer->name());
            action->setIcon(QIcon::fromTheme(offer->icon()));
            action->setEnabled(enabled);
            connect(action, &QAction::triggered, this, [this, offer, which] {
                runService(offer, relevantUrls(which));
            });
        }

        // A personal command
        action = submenu->addAction(i18n("Open With..."));
        // XXX: action->setIcon( QIcon::fromTheme((*offerIt).second) );
        action->setEnabled(enabled);
        connect(action, &QAction::triggered, this, [this, which] {
            const QList<QUrl> urls = relevantUrls(which);

            auto *uiParent = MainWindow::Window::theMainWindow();
            auto job = new KIO::ApplicationLauncherJob();
            job->setUrls(urls);
            job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, uiParent));
            job->start();
        });

        // A personal command
        // XXX: see kdialog.h for simple usage
        action = submenu->addAction(i18n("Your Command Line"));
        // XXX: action->setIcon( QIcon::fromTheme((*offerIt).second) );
        action->setEnabled(enabled);
        connect(action, &QAction::triggered, this, [this] {
            static RunDialog *dialog = new RunDialog(MainWindow::Window::theMainWindow());
            dialog->setImageList(m_list);
            dialog->show();
        });
    }
}

QList<QUrl> MainWindow::ExternalPopup::relevantUrls(PopupAction which)
{
    if (which == PopupAction::OpenCurrent) {
        return { QUrl(m_currentInfo->fileName().absolute()) };
    }

    if (which == PopupAction::OpenAllSelected) {
        QList<QUrl> lst;
        for (const DB::FileName &file : std::as_const(m_list)) {
            lst.append(QUrl(file.absolute()));
        }
        return lst;
    }

    if (which == PopupAction::CopyAndOpenAllSelected) {
        QList<QUrl> lst;
        // the way things are presented in the UI, a user is likely to expect this option to
        // copy and open all selected, not just one. Also, the computation of `enabled` in populate()
        // suggests the same (hence the enum name) ->
        // FIXME(jzarl) make this work on the file list!
        QString origFile = m_currentInfo->fileName().absolute();
        QString newFile = origFile;

        QString origRegexpString = Settings::SettingsData::instance()->copyFileComponent();
        auto origRegexp = QRegularExpression(origRegexpString);
        QString copyFileReplacement = Settings::SettingsData::instance()->copyFileReplacementComponent();

        if (origRegexpString.length() > 0) {
            newFile.replace(origRegexp, copyFileReplacement);
            QFile::copy(origFile, newFile);
            lst.append(QUrl::fromLocalFile(newFile));
        } else {
            qCWarning(MainWindowLog, "No settings were appropriate for modifying the file name (you must fill in the regexp field; Opening the original instead");
            lst.append(QUrl::fromLocalFile(origFile));
        }
        return lst;
    }
    return {};
}

void MainWindow::ExternalPopup::runService(KService::Ptr service, QList<QUrl> urls)
{
    auto *uiParent = MainWindow::Window::theMainWindow();
    auto job = new KIO::ApplicationLauncherJob(service);
    job->setUrls(urls);
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, uiParent));
    job->start();
}

MainWindow::ExternalPopup::ExternalPopup(QWidget *parent)
    : QMenu(parent)
{
    setTitle(i18n("Invoke External Program"));
}

QString MainWindow::ExternalPopup::mimeType(const DB::FileName &file)
{
    QMimeDatabase db;
    return db.mimeTypeForFile(file.absolute(), QMimeDatabase::MatchExtension).name();
}

Utilities::StringSet MainWindow::ExternalPopup::mimeTypes(const DB::FileNameList &files)
{
    Utilities::StringSet res;
    Utilities::StringSet extensions;
    for (const DB::FileName &file : files) {
        const DB::FileName baseFileName = file;
        const int extStart = baseFileName.relative().lastIndexOf(QChar::fromLatin1('.'));
        const QString ext = baseFileName.relative().mid(extStart);
        if (!extensions.contains(ext)) {
            res.insert(mimeType(file));
            extensions.insert(ext);
        }
    }
    return res;
}

namespace
{
KService::List getServiceOffers(const QString &type)
{
    return KApplicationTrader::queryByMimeType(type);
}
}

KService::List MainWindow::ExternalPopup::appInfos(const DB::FileNameList &files)
{
    const auto types = mimeTypes(files);
    if (types.isEmpty())
        return {};

    auto typesIt = types.cbegin();

    // get offers for first mimeType...
    const auto initialOffers = getServiceOffers(*typesIt);
    QSet<QString> commonOffers;
    for (const auto &offer : initialOffers) {
        commonOffers += offer->name();
    }

    // ... and eliminate any offer that does not support all files
    while (++typesIt != types.cend()) {
        const auto offers = getServiceOffers(*typesIt);
        QSet<QString> offerSet;
        for (const auto &offer : offers) {
            offerSet += offer->name();
        }
        commonOffers &= offerSet;
    }

    KService::List result;
    for (const auto &offer : initialOffers) {
        // KService::Ptrs will be different for different mime types, so we have to compare by name
        if (commonOffers.contains(offer->name()))
            result << offer;
    }

    return result;
}

bool operator<(const QPair<QString, QPixmap> &a, const QPair<QString, QPixmap> &b)
{
    return a.first < b.first;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ExternalPopup.cpp"
