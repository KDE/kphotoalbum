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
