/*
  Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>

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

#ifndef KINPUTDIALOG_H
#define KINPUTDIALOG_H

#include <klineeditdlg.h>

namespace KimDaBaCompat {

/**
 * The KInputDialog class provides a simple dialog to get a single value
 * from the user. The value can be a string, a number (either an integer or
 * a float) or an item from a list. This class is designed to be source
 * compatible with QInputDialog.
 *
 * Five static convenience functions are provided: getText(), getInteger().
 * getDouble(), getItem() and getItemList().
 *
 * @since 3.2
 * @author Nadeem Hasan <nhasan@kde.org>
 */
class KInputDialog : public KLineEditDlg
{
    Q_OBJECT
private:
    KInputDialog() :KLineEditDlg( QString::null, QString::null, (QWidget*) 0 ) {}
};

} // namespace KimDaBaCompat

using namespace KimDaBaCompat;

#endif // KINPUTDIALOG_H
