#ifndef CATEGORYLISTVIEW_DRAGABLELISTVIEW_H
#define CATEGORYLISTVIEW_DRAGABLELISTVIEW_H

#include <qlistview.h>

namespace CategoryListView
{
class DragableListView :public QListView
{
    Q_OBJECT

public:
    DragableListView( const QString& category, QWidget* parent, const char* name = 0 );
    QString category() const;
    void emitItemsChanged();

signals:
    void itemsChanged();

protected:
    virtual QDragObject* dragObject();

private:
    const QString _category;
};

};

#endif /* CATEGORYLISTVIEW_DRAGABLELISTVIEW_H */

