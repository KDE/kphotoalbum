#ifndef OVERVIEWPAGE_H
#define OVERVIEWPAGE_H
#include "Breadcrumb.h"
#include <AnnotationDialog/Dialog.h>
#include "BrowserPage.h"
#include <DB/ImageSearchInfo.h>
#include <DB/Category.h>
#include <QAbstractListModel>
#include <DB/MediaCount.h>

namespace Browser {
class BrowserWidget;

/**
 * \brief The overview page in the browser (the one containing People, Places, Show Images etc)
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 */
class OverviewPage :public QAbstractListModel, public BrowserPage
{
public:
    OverviewPage( const Breadcrumb& breadcrumb, const DB::ImageSearchInfo& info, Browser::BrowserWidget* );
    OVERRIDE int rowCount ( const QModelIndex& parent = QModelIndex() ) const;
    OVERRIDE QVariant data ( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    OVERRIDE void activate();
    OVERRIDE BrowserPage* activateChild( const QModelIndex& );
    OVERRIDE Qt::ItemFlags flags ( const QModelIndex & index ) const;
    OVERRIDE bool isSearchable() const;
    OVERRIDE Breadcrumb breadcrumb() const;
    OVERRIDE bool showDuringBack() const;


private:
    QList<DB::CategoryPtr> categories() const;

    bool isCategoryIndex( int row ) const;
    bool isExivIndex( int row ) const;
    bool isSearchIndex( int row ) const;
    bool isImageIndex( int row ) const;

    QVariant categoryInfo( int row, int role ) const;
    QVariant exivInfo( int role ) const;
    QVariant searchInfo( int role ) const;
    QVariant imageInfo( int role ) const;

    BrowserPage* activateExivAction();
    BrowserPage* activateSearchAction();

private:
    QMap<int,DB::MediaCount> _count;
    static AnnotationDialog::Dialog* _config;
    Breadcrumb _breadcrumb;
};

}


#endif /* OVERVIEWPAGE_H */

