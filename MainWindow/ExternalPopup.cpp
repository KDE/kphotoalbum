/* Copyright (C) 2003-2020 Jesper K. Pedersen <blackie@kde.org>

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

    QStringList list = QStringList() << i18n("Current Item") << i18n("All Selected Items") << i18n("Copy and Open");
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

        for (OfferType::const_iterator offerIt = offers.begin(); offerIt != offers.end(); ++offerIt) {
            action = submenu->addAction((*offerIt).first);
            action->setObjectName((*offerIt).first); // Notice this is needed to find the application later!
            action->setIcon(QIcon::fromTheme((*offerIt).second));
            action->setData(which);
            action->setEnabled(enabled);
        }

        // A personal command
        action = submenu->addAction(i18n("Open With..."));
        action->setObjectName(i18n("Open With...")); // Notice this is needed to find the application later!
        // XXX: action->setIcon( QIcon::fromTheme((*offerIt).second) );
        action->setData(which);
        action->setEnabled(enabled);

        // A personal command
        // XXX: see kdialog.h for simple usage
        action = submenu->addAction(i18n("Your Command Line"));
        action->setObjectName(i18n("Your Command Line")); // Notice this is needed to find the application later!
        // XXX: action->setIcon( QIcon::fromTheme((*offerIt).second) );
        action->setData(which);
        action->setEnabled(enabled);
    }
}

void MainWindow::ExternalPopup::slotExecuteService(QAction *action)
{
    QString name = action->objectName();
    const StringSet apps = m_appToMimeTypeMap[name];

    // get the list of arguments
    QList<QUrl> lst;

    if (action->data() == -1) {
        return; //user clicked the title entry. (i.e: "All Selected Items")
    } else if (action->data() == 1) {
        for (const DB::FileName &file : qAsConst(m_list)) {
            if (m_appToMimeTypeMap[name].contains(mimeType(file)))
                lst.append(QUrl(file.absolute()));
        }
    } else if (action->data() == 2) {
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

    } else {
        lst.append(QUrl(m_currentInfo->fileName().absolute()));
    }

    // get the program to run

    // check for the special entry for self-defined
    if (name == i18n("Your Command Line")) {

        static RunDialog *dialog = new RunDialog(MainWindow::Window::theMainWindow());
        dialog->setImageList(m_list);
        dialog->show();

        return;
    }

    auto *uiParent = MainWindow::Window::theMainWindow();
    // check for the special entry for self-defined
    if (name == i18n("Open With...")) {
#if KIO_VERSION < QT_VERSION_CHECK(5, 71, 0)
        KRun::displayOpenWithDialog(lst, uiParent);
#else
        auto job = new KIO::ApplicationLauncherJob();
        job->setUrls(lst);
        job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, uiParent));
        job->start();
#endif
        return;
    }

    KService::List offers = KMimeTypeTrader::self()->query(*(apps.begin()), QString::fromLatin1("Application"),
                                                           QString::fromLatin1("Name == '%1'").arg(name));
    Q_ASSERT(offers.count() >= 1);
    KService::Ptr service = offers.first();
#if KIO_VERSION < QT_VERSION_CHECK(5, 70, 0)
    KRun::runService(*service, lst, uiParent);
#else
    auto job = new KIO::ApplicationLauncherJob(service);
    job->setUrls(lst);
    job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, uiParent));
    job->start();
#endif
}

MainWindow::ExternalPopup::ExternalPopup(QWidget *parent)
    : QMenu(parent)
{
    setTitle(i18n("Invoke External Program"));
    connect(this, &ExternalPopup::triggered, this, &ExternalPopup::slotExecuteService);
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
            res.insert(qMakePair(offer->name(), offer->icon()));
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
