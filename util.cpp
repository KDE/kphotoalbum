#include "util.h"
#include "options.h"
#include "imageinfo.h"

bool Util::writeOptions( QDomDocument doc, QDomElement elm, QMap<QString, QStringList>& options )
{
    bool anyAtAll = false;
    for( QMapIterator<QString,QStringList> it= options.begin(); it != options.end(); ++it ) {
        QDomElement opt = doc.createElement( "option" );
        opt.setAttribute( "name",  it.key() );
        QStringList list = it.data();
        bool any = false;
        for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
            QDomElement val = doc.createElement( "value" );
            val.setAttribute( "value", *it2 );
            opt.appendChild( val );
            any = true;
            anyAtAll = true;
        }
        if ( any )
            elm.appendChild( opt );
    }
    return anyAtAll;
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

QString Util::createInfoText( ImageInfo* info )
{
    QString text = "" ;
    if ( Options::instance()->showDate() )  {
        if ( info->startDate().isNull() )
            text += "";
        else if ( info->endDate().isNull() )
            text += info->startDate();
        else
            text += info->startDate() + " to " + info->endDate();

        if ( !text.isEmpty() ) {
            text = "<b>Date:</b> " + text + "<br>";
        }
    }

    // PENDING(blackie) The key is used both as a key and a label, which is a problem here.
    if ( Options::instance()->showLocation() )  {
        QString location = info->optionValue( "Locations" ).join( ", " );
        if ( !location.isEmpty() )
            text += "<b>Location:</b> " + location + "<br>";
    }

    if ( Options::instance()->showNames() ) {
        QString persons = info->optionValue( "Persons" ).join( ", " );
        if ( !persons.isEmpty() )
            text += "<b>Persons:</b> " + persons + "<br>";
    }

    if ( Options::instance()->showDescription() && !info->description().isEmpty())  {
        if ( !text.isEmpty() )
            text += "<b>Description:</b> " +  info->description() + "<br>";
    }

    if ( Options::instance()->showKeyWords() )  {
        QString keyWords = info->optionValue( "Keywords" ).join( ", " );
        if ( !keyWords.isEmpty() )
            text += "<b>Key Words:</b> " + keyWords + "<br>";
    }
    return text;
}
