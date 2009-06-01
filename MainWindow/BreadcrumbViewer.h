#ifndef BREADCRUMBVIEWER_H
#define BREADCRUMBVIEWER_H
#include <QLabel>
#include <Browser/BreadcrumbList.h>

class BreadcrumbViewer :public QLabel
{
    Q_OBJECT

public:
    BreadcrumbViewer();

public slots:
    void setBreadcrumbs( const Browser::BreadcrumbList& list );

signals:
    void widenToBreadcrumb( const Browser::Breadcrumb& );

private slots:
    void linkClicked( const QString& link );

private:
    Browser::BreadcrumbList _activeCrumbs;
};

#endif /* BREADCRUMBVIEWER_H */

