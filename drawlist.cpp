#include "drawlist.h"
#include "draw.h"
#include "linedraw.h"
#include "rectdraw.h"
#include "circledraw.h"
DrawList::DrawList() : QValueList<Draw*>()
{
}

DrawList& DrawList::operator=( const DrawList& other )
{
    if ( this == &other )
        return *this;
    deleteItems();

    for( QValueList<Draw*>::ConstIterator it = other.begin(); it != other.end(); ++it ) {
        append( (*it)->clone() );
    }
    return *this;
}

DrawList::~DrawList()
{
    deleteItems();
}

void DrawList::deleteItems()
{
    for( QValueList<Draw*>::ConstIterator it = begin(); it != end();  ) {
        Draw* item = *it;
        ++it;
        delete item;
    }
    clear();
}

DrawList::DrawList( const DrawList& other )
    : QValueList<Draw*>()
{
    for( QValueList<Draw*>::ConstIterator it = other.begin(); it != other.end(); ++it ) {
        append( (*it)->clone() );
    }
}

void DrawList::load( QDomElement elm )
{
    Q_ASSERT( elm.tagName() == QString::fromLatin1( "drawings" ) );
    Q_ASSERT( count() == 0 );
    for ( QDomNode node = elm.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( !node.isElement() )
            continue;
        QDomElement child = node.toElement();

        QString tag = child.tagName();
        if ( tag == QString::fromLatin1( "Line" ) ) {
            append( new LineDraw( child ) );
        }
        else if ( tag == QString::fromLatin1( "Rectangle" ) ) {
            append( new RectDraw( child ) );
        }
        else if ( tag == QString::fromLatin1( "Circle" ) ) {
            append( new CircleDraw( child ) );
        }
        else {
            // PENDING(blackie) Do it the KDE way
            qWarning("Unexpected tag: %s", tag.latin1() );
        }
    }
}

void DrawList::save( QDomDocument doc, QDomElement top )
{
    if ( count() == 0 )
        return;

    QDomElement elm = doc.createElement( QString::fromLatin1( "drawings" ) );
    top.appendChild( elm );

    for( QValueList<Draw*>::iterator it = begin(); it != end(); ++it ) {
        elm.appendChild( (*it)->save( doc ) );
    }
}

void DrawList::setWidget( QWidget* widget )
{
    for( QValueList<Draw*>::ConstIterator it = begin(); it != end(); ++it ) {
        (*it)->setWidget( widget );
    }
}
