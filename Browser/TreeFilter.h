#ifndef TREEFILTER_H
#define TREEFILTER_H
#include <QSortFilterProxyModel>

/**
 * \brief Filter proxy that keeps parent branches if child branches matches
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 * The QSortFilterProxyModel has one drawback that makes it inappropriate to
 * use for filtering in the browser, namely that it hides an item, if the item
 * doesn't match the filter criteria, even if child items actually do match
 * the filter criteria. This class overcomes this shortcoming.
 */
class TreeFilter :public QSortFilterProxyModel
{
public:
    TreeFilter( QObject* parent = 0 );
    void resetCache();

protected:
    virtual bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const;

    mutable QMap<QModelIndex,bool> _matchedMap;
};


#endif /* TREEFILTER_H */

