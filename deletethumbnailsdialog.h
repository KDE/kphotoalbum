/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef DELETEFILELISTDIALOG_H
#define DELETEFILELISTDIALOG_H

#include <kdialogbase.h>
#include <qstringlist.h>

class DeleteThumbnailsDialog :public KDialogBase {
    Q_OBJECT

public:
    DeleteThumbnailsDialog( QWidget* parent, const char* name = 0 );

protected slots:
    void slotDeleteFiles();
    void findThumbnails( const QString& directory );

private:
    QStringList _files;
};


#endif /* DELETEFILELISTDIALOG_H */

