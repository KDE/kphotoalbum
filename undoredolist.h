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

