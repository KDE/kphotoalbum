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

#ifndef UNDOREDOLIST_H
#define UNDOREDOLIST_H
#include "undoredoobject.h"
#include <qobject.h>
#include <qvaluelist.h>

// This class is not yet in use - I expect to use it when adding undo/redo.
class UndoRedoList :public QObject {
    Q_OBJECT

public:
    UndoRedoList( QObject* parent, const char* name );
    void add( UndoRedoObject* );
    void forward();
    void backward();

signals:
    void canUndo( bool );
    void canRedo( bool );

private:
    int _current;
    QValueList<UndoRedoObject*> _list;
};


#endif /* UNDOREDOLIST_H */

