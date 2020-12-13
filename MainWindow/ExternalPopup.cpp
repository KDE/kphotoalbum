/* SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

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

    const QStringList list = QStringList() << i18n("Current Item") << i18n("All Selected Items") << i18n("Copy and Open");
    for (int which = 0; which < 3; ++which) {
        if (which == 0 && !current)
            continue;

        const bool multiple = (m_list.count() > 1);
        const bool enabled = (which != 1 && m_currentInfo) || (which == 1 && multiple);

        // Submenu
        QMenu *submenu = addMenu(list[which]);
        submenu->setEnabled(enabled);

        // Fetch set of offers
        OfferType offers;
        if (which == 0)
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

QList<QUrl> MainWindow::ExternalPopup::relevantUrls(int which, KService::Ptr service)
{
    // "current item"
    if (which == 0) {
        return {QUrl(m_currentInfo->fileName().absolute())};
    }

    // "all selected"
    if (which == 1) {
        QList<QUrl> lst;
        for (const DB::FileName &file : qAsConst(m_list)) {
            if (service && m_appToMimeTypeMap[service->name()].contains(mimeType(file)))
                lst.append(QUrl(file.absolute()));
        }
        return lst;
    }

    // "copy and open"
    if (which == 2) {
        QList<QUrl> lst;
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
        const KService::List offers = KMimeTypeTrader::self()->query(type, QLatin1String("Application"));
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
