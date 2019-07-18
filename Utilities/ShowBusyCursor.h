/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef SHOWBUSYCURSOR_H
#define SHOWBUSYCURSOR_H

#include <qnamespace.h>

namespace Utilities
{

/**
   \brief Utility class to set/unset the busy cursor

   When setting the busy cursor, you also need to remember to unset it
   again, otherwise you will have a busy cursor for the rest of the
   lifetime of the application.

   This class helps you avoid not getting it unset due to an early return
   in a function (much similar to \ref BooleanGuard). The code looks like
   this:

   <pre>
   void f() {
       ...
       ShowBusyCursor dummy;
       ... // cursor will be busy until the end of this function.
   }
   </pre>
**/

class ShowBusyCursor
{

public:
    explicit ShowBusyCursor(Qt::CursorShape shape = Qt::WaitCursor);
    ~ShowBusyCursor();
    void stop();

private:
    bool m_active;
};
}

#endif /* SHOWBUSYCURSOR_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
