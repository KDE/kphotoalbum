#ifndef BREADCRUMBVIEWER_H
#define BREADCRUMBVIEWER_H
#include <QLabel>
#include <Browser/BreadcrumbList.h>

class BreadcrumbViewer :public QLabel
{
    Q_OBJECT

public:
    BreadcrumbViewer();
    OVERRIDE QSize minimumSizeHint() const;

public slots:
    void setBreadcrumbs( const Browser::BreadcrumbList& list );

signals:
    void widenToBreadcrumb( const Browser::Breadcrumb& );

protected:
    OVERRIDE void resizeEvent( QResizeEvent* event );

private slots:
    void linkClicked( const QString& link );

private:
    void updateText();

private:
    Browser::BreadcrumbList _activeCrumbs;
};

#endif /* BREADCRUMBVIEWER_H */

