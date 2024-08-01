/* SPDX-FileCopyrightText: 2016-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "Options.h"

#include "Logging.h"

#include <KLocalizedString>
#include <QCommandLineOption>
#include <QCommandLineParser>

MainWindow::Options *MainWindow::Options::s_instance = nullptr;

namespace MainWindow
{
class Options::OptionsPrivate
{
public:
    QCommandLineParser parser;

    // legacy option: "-c <imageDirectory>"
    QCommandLineOption configFile {
        QLatin1String("c"),
        i18n("Use <databaseFile> instead of the default. Deprecated - use '--db <databaseFile>' instead."),
        i18n("databaseFile")
    };
    QCommandLineOption dbFile {
        QLatin1String("db"),
        i18n("Use <databaseFile> instead of the default."),
        i18n("databaseFile")
    };
    QCommandLineOption demoOption { QLatin1String("demo"), i18n("Starts KPhotoAlbum with a prebuilt set of demo images.") };
    QCommandLineOption importFile {
        QLatin1String("import"),
        i18n("Import file."),
        i18n("file.kim")
    };
#ifdef KPA_ENABLE_REMOTECONTROL
    // QCommandLineParser doesn't support optional values.
    // therefore, we need two separate options:
    QCommandLineOption listen {
        QLatin1String("listen"),
        i18n("Listen for network connections.")
    };
    QCommandLineOption listenAddress {
        QLatin1String("listen-address"),
        i18n("Listen for network connections on address <interface_address>."),
        i18n("interface_address")
    };
#endif
    QCommandLineOption searchOnStartup { QLatin1String("search"), i18n("Search for new images on startup.") };
};
}

MainWindow::Options *MainWindow::Options::the()
{
    if (!s_instance)
        s_instance = new Options();
    return s_instance;
}

QCommandLineParser *MainWindow::Options::parser() const
{
    return &(d->parser);
}

QUrl MainWindow::Options::dbFile() const
{
    QUrl db;
    if (d->parser.isSet(d->dbFile)) {
        db = QUrl::fromLocalFile(d->parser.value(d->dbFile));
    } else if (d->parser.isSet(d->configFile)) {
        // support for legacy option
        db = QUrl::fromLocalFile(d->parser.value(d->configFile));
    }
    return db;
}

bool MainWindow::Options::demoMode() const
{
    return d->parser.isSet(d->demoOption);
}

QUrl MainWindow::Options::importFile() const
{
    if (d->parser.isSet(d->importFile))
        return QUrl::fromLocalFile(d->parser.value(d->importFile));
    return QUrl();
}

QHostAddress MainWindow::Options::listen() const
{
#ifdef KPA_ENABLE_REMOTECONTROL
    QHostAddress address;
    QString value = d->parser.value(d->listenAddress);
    if (d->parser.isSet(d->listen) || !value.isEmpty()) {
        if (value.isEmpty())
            address = QHostAddress::Any;
        else
            address = QHostAddress(value);
    }
    if (address.isMulticast() || address == QHostAddress::Broadcast) {
        qCWarning(MainWindowLog) << "Won't bind to address" << address;
        address = QHostAddress::Null;
    }
    return address;
#else
    return {};
#endif
}

bool MainWindow::Options::searchForImagesOnStart() const
{
    return d->parser.isSet(d->searchOnStartup);
}

MainWindow::Options::Options()
    : d(new OptionsPrivate)
{
    d->configFile.setFlags(QCommandLineOption::HiddenFromHelp);
    d->parser.addOptions(
        QList<QCommandLineOption>()
        << d->configFile
        << d->dbFile
        << d->demoOption
        << d->importFile
#ifdef KPA_ENABLE_REMOTECONTROL
        << d->listen
        << d->listenAddress
#endif
        << d->searchOnStartup);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
