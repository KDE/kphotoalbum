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
#ifndef OPTIONS_H
#define OPTIONS_H

#include <QHostAddress>
#include <QScopedPointer>
#include <QUrl>
class QCommandLineParser;

namespace MainWindow
{

/**
 * @brief The Options class is a simple wrapper around QCommandLineParser that makes the command line arguments available after startup.
 */
class Options
{
public:
    static Options *the();

    /**
     * @brief Gives direct access to the QCommandLineParser object.
     * This is for main().
     * @return
     */
    QCommandLineParser *parser() const;

    QUrl dbFile() const;
    /**
     * @brief demoMode
     * @return true, if demo mode is set.
     */
    bool demoMode() const;
    /**
     * @brief importFile
     * @return the QUrl of the import file, or an empty QUrl, if no import file is set.
     */
    QUrl importFile() const;
    /**
     * @brief listen
     * Access the address that was givent to the commandline "--listen" argument.
     * @return a null address if listening is disabled, otherwise an address to bind to.
     */
    QHostAddress listen() const;
    /**
     * @brief searchOnStartup
     * @return true, if we want to search for images on startup
     */
    bool searchForImagesOnStart() const;

private:
    class OptionsPrivate;
    Options();

    static Options *s_instance;
    const QScopedPointer<OptionsPrivate> d;
};

}

#endif /* OPTIONS_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
