#ifndef LISTVIEWITEMHIDER_H
#define LISTVIEWITEMHIDER_H
#include <qlistview.h>

namespace AnnotationDialog {

class ListViewHider
{
protected:
    ListViewHider() {}
    void setItemsVisible( QListView* );
    bool setItemsVisible( QListViewItem* parentItem );
    virtual bool shouldItemBeShown( QListViewItem* ) = 0;
};


class ListViewTextMatchHider :public ListViewHider
{
public:
    ListViewTextMatchHider( const QString& text, bool anchorAtStart, QListView* listView );

protected:
    virtual bool shouldItemBeShown( QListViewItem* );

private:
    QString _text;
    bool _anchorAtStart;
};

class ListViewCheckedHider :public ListViewHider
{
public:
    ListViewCheckedHider( QListView* );

protected:
    virtual bool shouldItemBeShown( QListViewItem* );
};

}

#endif /* LISTVIEWITEMHIDER_H */

