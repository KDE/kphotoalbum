#include "BreadcrumbViewer.h"
#include <QDebug>

void BreadcrumbViewer::setBreadcrumbs( const Browser::BreadcrumbList& list )
{
    _activeCrumbs = list.latest();
    QStringList tmp;

    for ( int i = 0; i < _activeCrumbs.count()-1; ++i )
        tmp.append( QString::fromLatin1("<a href=\"%1\">%2</a>").arg(i).arg(_activeCrumbs[i].text()) );
    tmp.append(_activeCrumbs[_activeCrumbs.count()-1].text());

    QString text = tmp.join(QString::fromLatin1(" > ") );

    setText( text );
}

void BreadcrumbViewer::linkClicked( const QString& link )
{
    emit widenToBreadcrumb( _activeCrumbs[ link.toInt() ] );
}

BreadcrumbViewer::BreadcrumbViewer()
{
    connect( this, SIGNAL( linkActivated( QString ) ), this, SLOT( linkClicked( QString ) ) );
}
