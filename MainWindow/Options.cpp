/* Copyright (C) 2016 Johannes Zarl-Zierl <johannes@zarl-zierl.at>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    d->parser.addOptions({ d->configFile, d->demoOption, d->importFile });
}


// vi:expandtab:tabstop=4 shiftwidth=4:
