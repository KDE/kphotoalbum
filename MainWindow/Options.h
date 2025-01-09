/*
 * SPDX-FileCopyrightText: 2016 - 2025 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

    /**
     * @brief config
     *
     * Note: this value is not directly used in kphotoalbum, but by KSharedConfig::openConfig.
     * It needs to be allowed (and documented) as a commandline option here so that it is available for KConfig::mainConfigName() to use.
     *
     * @return the path to kphotoalbumrc
     */
    QString config() const;

    /**
     * @brief dbFile
     * @return the XML database file specified on the command line, if any.
     */
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
