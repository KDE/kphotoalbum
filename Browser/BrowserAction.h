#ifndef BROWSERMODEL_H
#define BROWSERMODEL_H
#include <DB/Category.h>
#include <QAbstractItemModel>

namespace Browser
{
class BrowserWidget;

enum Viewer { ShowBrowser, ShowImageViewer };

class BrowserAction
{
public:
    BrowserAction( BrowserWidget* browser );
    virtual ~BrowserAction() {}
    virtual void activate() = 0;
    virtual BrowserAction* generateChildAction( const QModelIndex &);
    BrowserWidget* browser() const;
    virtual Viewer viewer();
    virtual DB::Category::ViewType viewType() const;
    virtual bool isSearchable() const;
    virtual bool isViewChangeable() const;

private:
    BrowserWidget* _browser;
};

}


#endif /* BROWSERMODEL_H */

