/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
