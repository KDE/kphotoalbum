// SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2020-2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ExternalPopup.h"

#include "Logging.h"
#include "RunDialog.h"
#include "Window.h"

#include <DB/FileNameList.h>
#include <DB/ImageInfo.h>
#include <Settings/SettingsData.h>

#include <KFileItem>
#include <KLocalizedString>
#include <KMimeTypeTrader>
#include <kio_version.h>
#if KIO_VERSION >= QT_VERSION_CHECK(5, 70, 0)
#include <KIO/ApplicationLauncherJob>
#include <KIO/JobUiDelegate>
#endif
#if KIO_VERSION < QT_VERSION_CHECK(5, 71, 0)
// KRun::displayOpenWithDialog() was both replaced and deprecated in 5.71
#include <KRun>
#endif
#include <kservice_version.h>
#if KSERVICE_VERSION >= QT_VERSION_CHECK(5, 68, 0)
#include <KApplicationTrader>
#endif
#include <KService>
#include <KShell>
#include <QFile>
#include <QIcon>
#include <QLabel>
#include <QMimeDatabase>
#include <QPixmap>
#include <QStringList>
#include <QUrl>

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
        OfferType offers;
        if (which == PopupAction::OpenCurrent)
            offers = appInfos(DB::FileNameList() << current->fileName());
        else
            offers = appInfos(imageList);

        for (KService::Ptr offer : qAsConst(offers)) {
            action = submenu->addAction(offer->name());
            action->setIcon(QIcon::fromTheme(offer->icon()));
            action->setEnabled(enabled);
            connect(action, &QAction::triggered, this, [this, offer, which] {
                runService(offer, relevantUrls(which, offer));
            });
        }

        // A personal command
        action = submenu->addAction(i18n("Open With..."));
        // XXX: action->setIcon( QIcon::fromTheme((*offerIt).second) );
        action->setEnabled(enabled);
        connect(action, &QAction::triggered, this, [this, which] {
            const QList<QUrl> urls = relevantUrls(which, {});

            auto *uiParent = MainWindow::Window::theMainWindow();
#if KIO_VERSION < QT_VERSION_CHECK(5, 71, 0)
            KRun::displayOpenWithDialog(lst, uiParent);
#else
                auto job = new KIO::ApplicationLauncherJob();
                job->setUrls(urls);
                job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, uiParent));
                job->start();
#endif
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

QList<QUrl> MainWindow::ExternalPopup::relevantUrls(PopupAction which, KService::Ptr service)
{
    if (which == PopupAction::OpenCurrent) {
        return { QUrl(m_currentInfo->fileName().absolute()) };
    }

    if (which == PopupAction::OpenAllSelected) {
        QList<QUrl> lst;
        for (const DB::FileName &file : qAsConst(m_list)) {
            if (service && m_appToMimeTypeMap[service->name()].contains(mimeType(file)))
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
        QRegExp origRegexp = QRegExp(origRegexpString);
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
#if KIO_VERSION < QT_VERSION_CHECK(5, 70, 0)
    KRun::runService(*service, urls, uiParent);
#else
    auto job = new KIO::ApplicationLauncherJob(service);
    job->setUrls(urls);
    job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, uiParent));
    job->start();
#endif
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
    StringSet res;
    StringSet extensions;
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

MainWindow::OfferType MainWindow::ExternalPopup::appInfos(const DB::FileNameList &files)
{
    const StringSet types = mimeTypes(files);
    OfferType res;
    for (const QString &type : types) {
#if KSERVICE_VERSION < QT_VERSION_CHECK(5, 68, 0)
        const KService::List offers = KMimeTypeTrader::self()->query(type, QLatin1String("Application"));
#else
        const KService::List offers = KApplicationTrader::queryByMimeType(type);
#endif
        for (const KService::Ptr &offer : offers) {
            res.insert(offer);
            m_appToMimeTypeMap[offer->name()].insert(type);
        }
    }
    return res;
}

bool operator<(const QPair<QString, QPixmap> &a, const QPair<QString, QPixmap> &b)
{
    return a.first < b.first;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
