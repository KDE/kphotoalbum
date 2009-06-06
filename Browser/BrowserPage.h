#ifndef BROWSERPAGE_H
#define BROWSERPAGE_H
#include "Breadcrumb.h"
#include <DB/ImageSearchInfo.h>
#include <DB/Category.h>
#include <QAbstractItemModel>

namespace Browser
{
class BrowserWidget;

enum Viewer { ShowBrowser, ShowImageViewer };

/**
 */
class BrowserPage
{
public:
     BrowserPage( const DB::ImageSearchInfo& info, BrowserWidget* browser );
     virtual ~BrowserPage() {}

    /**
     * Activate the action. Result of activation may be to call \ref BrowserWidget::addAction.
     */
    virtual void activate() = 0;

    virtual BrowserPage* activateChild( const QModelIndex &);
    BrowserWidget* browser() const;
    virtual Viewer viewer();
    virtual DB::Category::ViewType viewType() const;
    virtual bool isSearchable() const;
    virtual bool isViewChangeable() const;
    DB::ImageSearchInfo searchInfo() const;
    virtual Breadcrumb breadcrumb() const;
    virtual bool showDuringBack() const;

private:
    DB::ImageSearchInfo _info;
    BrowserWidget* _browser;
};

}


#endif /* BROWSERPAGE_H */

