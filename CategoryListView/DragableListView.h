#ifndef CATEGORYLISTVIEW_DRAGABLELISTVIEW_H
#define CATEGORYLISTVIEW_DRAGABLELISTVIEW_H

#include <qlistview.h>
#include "DB/Category.h"

namespace CategoryListView
{
class DragableListView :public QListView
{
    Q_OBJECT

public:
    DragableListView( const DB::CategoryPtr& category, QWidget* parent, const char* name = 0 );
    DB::CategoryPtr category() const;
    void emitItemsChanged();

signals:
    void itemsChanged();

protected:
    virtual QDragObject* dragObject();

private:
    const DB::CategoryPtr _category;
};

};

#endif /* CATEGORYLISTVIEW_DRAGABLELISTVIEW_H */

