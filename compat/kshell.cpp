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

#include <kshell.h>

#include <qfile.h>

#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

QString KShell::tildeExpand( const QString &fname )
{
    if (fname[0] == '~') {
        int pos = fname.find( '/' );
        if (pos < 0)
            return homeDir( QConstString( fname.unicode() + 1, fname.length() - 1 ).string() );
        QString ret = homeDir( QConstString( fname.unicode() + 1, pos - 1 ).string() );
        if (!ret.isNull())
            ret += QConstString( fname.unicode() + pos, fname.length() - pos ).string();
        return ret;
    }
    return fname;
}

QString KShell::homeDir( const QString &user )
{
    if (user.isEmpty())
        return QFile::decodeName( getenv( "HOME" ) );
    struct passwd *pw = getpwnam( QFile::encodeName( user ).data() );
    if (!pw)
        return QString::null;
    return QFile::decodeName( pw->pw_dir );
}
