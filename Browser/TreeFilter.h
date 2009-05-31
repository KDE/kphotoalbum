#ifndef TREEFILTER_H
#define TREEFILTER_H
#include <QSortFilterProxyModel>

class TreeFilter :public QSortFilterProxyModel
{
public:
    TreeFilter( QObject* parent = 0 );

protected:
    virtual bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const;
};


#endif /* TREEFILTER_H */

