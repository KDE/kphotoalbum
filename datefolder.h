/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/
#ifndef DATEFOLDER_H
#define DATEFOLDER_H

#include "folder.h"
#include <kdialogbase.h>
#include <qdatetime.h>
class KDatePicker;

class DateFolder :public Folder {
public:
    DateFolder( const ImageSearchInfo& info, Browser* parent );
    virtual FolderAction* action( bool ctrlDown = false );
    virtual QPixmap pixmap();
    virtual QString text() const;

};

class DateSearchDialog :public KDialogBase
{
    Q_OBJECT
public:
    DateSearchDialog( QWidget* parent, const char* name = 0 );
    QDate fromDate() const;
    QDate toDate() const;

protected slots:
    void fromDateChanged( QDate );
    void toDateChanged();

private:
    KDatePicker* _from;
    KDatePicker* _to;
    bool _toChanged;
};


#endif /* DATEFOLDER_H */

