#include "util.h"

void Util::writeOptions( QDomDocument doc, QDomElement elm, QMap<QString, QStringList>& options )
{
    for( QMapIterator<QString,QStringList> it= options.begin(); it != options.end(); ++it ) {
        QDomElement opt = doc.createElement( "option" );
        opt.setAttribute( "name",  it.key() );
        elm.appendChild( opt );
        QStringList list = it.data();
        for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
            QDomElement val = doc.createElement( "value" );
            val.setAttribute( "value", *it2 );
            opt.appendChild( val );
        }
    }
}



void Util::readOptions( QDomElement elm, QMap<QString, QStringList>* options )
{
    Q_ASSERT( elm.tagName() == QString::fromLatin1( "Options" ) );
    for ( QDomNode nodeOption = elm.firstChild(); !nodeOption.isNull(); nodeOption = nodeOption.nextSibling() )  {
        if ( nodeOption.isElement() )  {
            QDomElement elmOption = nodeOption.toElement();
            Q_ASSERT( elmOption.tagName() == "option" );
            QString name = elmOption.attribute( "name" );
            if ( !name.isNull() )  {
                for ( QDomNode nodeValue = elmOption.firstChild(); !nodeValue.isNull(); nodeValue = nodeValue.nextSibling() ) {
                    if ( nodeValue.isElement() ) {
                        QDomElement elmValue = nodeValue.toElement();
                        Q_ASSERT( elmValue.tagName() == "value" );
                        QString value = elmValue.attribute( "value" );
                        if ( !value.isNull() )  {
                            (*options)[name].append( value );
                        }
                    }
                }
            }
        }
    }
}
