/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
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

#include "undoredolist.h"

void UndoRedoList::add( UndoRedoObject* obj)
{
    while ( (int) _list.count() > _current )
        _list.pop_back();

    _list.append(obj);
    forward();
}

void UndoRedoList::forward()
{
    _list[_current]->redo();
    _current++;
    emit canUndo( _current != 1 );
    emit canRedo( _current < (int)_list.count() );
}

void UndoRedoList::backward()
{
    _current--;
    _list[_current]->undo();
    emit canUndo( _current != 1 );
    emit canRedo( _current < (int) _list.count() );
}

UndoRedoList::UndoRedoList( QObject* parent, const char* name )
    :QObject( parent, name ), _current(0)
{
}

#include "undoredolist.moc"
