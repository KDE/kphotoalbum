#ifndef BREADCRUMBLIST_H
#define BREADCRUMBLIST_H
#include <QStringList>
#include "Breadcrumb.h"

namespace Browser
{

/**
 * \brief A List of \ref Breadcrumb's
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 */
class BreadcrumbList :public QList<Breadcrumb>
{
public:
    BreadcrumbList latest() const;
    QString toString() const;
};

}

#endif /* BREADCRUMBLIST_H */

