#ifndef UNDOREDOOBJECT_H
#define UNDOREDOOBJECT_H

// This class is not yet in use - I expect to use it when adding undo/redo.
class UndoRedoObject {

public:
    virtual void redo() = 0;
    virtual void undo() = 0;
};


#endif /* UNDOREDOOBJECT_H */

