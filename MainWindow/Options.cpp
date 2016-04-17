/* Copyright (C) 2016 Johannes Zarl-Zierl <johannes@zarl-zierl.at>

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
#include "Options.h"

#include <QCommandLineOption>
#include <QCommandLineParser>

#include <KLocalizedString>

MainWindow::Options* MainWindow::Options::s_instance = nullptr;

namespace MainWindow {
class Options::OptionsPrivate {
public:
    QCommandLineParser parser;

    QCommandLineOption configFile {QLatin1String("c "), i18n("Config file")};
    QCommandLineOption demoOption {QLatin1String("demo"), i18n( "Starts KPhotoAlbum with a prebuilt set of demo images." )};
    QCommandLineOption importFile {QLatin1String("import "), i18n( "Import file." )};
    QCommandLineOption noListenNetworkOption { QLatin1String("nolisten-network"),
                i18n( "Don't start listening for android devices on startup." )};
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
    if (d->parser.isSet( d->configFile))
        return QUrl::fromLocalFile( d->parser.value( d->configFile ));
    return QUrl();
}

bool MainWindow::Options::demoMode() const
{
    return d->parser.isSet( d->demoOption );
}

QUrl MainWindow::Options::importFile() const
{
    if (d->parser.isSet( d->importFile))
        return QUrl::fromLocalFile( d->parser.value( d->importFile ));
    return QUrl();
}

MainWindow::Options::Options()
    : d(new OptionsPrivate)
{
    d->parser.addVersionOption();
    d->parser.addHelpOption();
    d->parser.addOptions({ d->configFile, d->demoOption, d->importFile, d->noListenNetworkOption});
}


// vi:expandtab:tabstop=4 shiftwidth=4:
