#ifndef BREADCRUMBLIST_H
#define BREADCRUMBLIST_H
#include <QStringList>
#include "Breadcrumb.h"

namespace Browser
{

class BreadcrumbList :public QList<Breadcrumb>
{
public:
    QStringList latest() const;
    QString toString() const;
};

}

#endif /* BREADCRUMBLIST_H */

