/*
    This file is part of the KDE libraries

    Copyright (c) 2003 Oswald Buddenhagen <ossi@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef _KSHELL_H
#define _KSHELL_H

#include <qstring.h>
#include <qstringlist.h>

namespace KimDaBaCompat {

/**
 * \namespace KShell
 * Provides some basic POSIX shell and bash functionality.
 * @see KStringHandler
 */
namespace KShell {

    /**
     * Flags for splitArgs().
     */
    enum Options {
        NoOptions = 0,

        /**
         * Perform tilde expansion.
         */
        TildeExpand = 1,

        /**
         * Bail out if a non-quoting and not quoted shell meta character is encoutered.
         * Meta characters are the command separators @p semicolon and @p ampersand,
         * the redirection symbols @p less-than, @p greater-than and the @p pipe @p symbol,
         * the grouping symbols opening and closing @p parens and @p braces, the command
         * substitution symbol @p backquote, the generic substitution symbol @p dollar
         * (if not followed by an apostrophe), the wildcards @p asterisk and
         * @p question @p mark, and the comment symbol @p hash @p mark. Additionally,
         * a variable assignment in the first word is recognized.
         */
        AbortOnMeta = 2
    };

    /**
     * Status codes from splitArgs()
     */
    enum Errors {
        /**
         * Success.
         */
        NoError = 0,

        /**
         * Indicates a parsing error, like an unterminated quoted string.
         */
        BadQuoting,

        /**
         * The AbortOnMeta flag was set and a shell meta character
         * was encoutered.
         */
        FoundMeta
    };

    /**
     * Performs tilde expansion on @p path. Interprets "~/path" and
     * "~user/path".
     *
     * @param path the path to tilde-expand
     * @return the expanded path
     */
    QString tildeExpand( const QString &path );

    /**
     * Obtain a @p user's home directory.
     *
     * @param user The name of the user whose home dir should be obtained.
     *  An empty string denotes the current user.
     * @return The user's home directory.
     */
    QString homeDir( const QString &user );

}

} // namespace KimDaBaCompat

using namespace KimDaBaCompat;

#endif /* _KSHELL_H */
