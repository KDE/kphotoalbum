#ifndef BROWSERMODEL_H
#define BROWSERMODEL_H
#include <QAbstractItemModel>

namespace Browser
{
class BrowserWidget;

class BrowserAction
{
public:
    BrowserAction( BrowserWidget* browser );
    virtual ~BrowserAction() {}
    virtual void activate() = 0;
    virtual BrowserAction* generateChildAction( const QModelIndex &);
    BrowserWidget* browser() const;

private:
    BrowserWidget* _browser;
};

}


#endif /* BROWSERMODEL_H */

