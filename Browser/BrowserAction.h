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
    virtual QAbstractItemModel* model() = 0;
    virtual void action( const QModelIndex &) = 0;
    BrowserWidget* browser() const;

private:
    BrowserWidget* _browser;
};

}


#endif /* BROWSERMODEL_H */

