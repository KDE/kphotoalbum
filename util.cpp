#include "util.h"
#include "options.h"
#include "imageinfo.h"
#include <klocale.h>

bool Util::writeOptions( QDomDocument doc, QDomElement elm, QMap<QString, QStringList>& options )
{
    bool anyAtAll = false;
    for( QMapIterator<QString,QStringList> it= options.begin(); it != options.end(); ++it ) {
        QDomElement opt = doc.createElement( QString::fromLatin1("option") );
        opt.setAttribute( QString::fromLatin1("name"),  it.key() );
        QStringList list = it.data();
        bool any = false;
        for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
            QDomElement val = doc.createElement( QString::fromLatin1("value") );
            val.setAttribute( QString::fromLatin1("value"), *it2 );
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
            Q_ASSERT( elmOption.tagName() == QString::fromLatin1("option") );
            QString name = elmOption.attribute( QString::fromLatin1("name") );
            if ( !name.isNull() )  {
                for ( QDomNode nodeValue = elmOption.firstChild(); !nodeValue.isNull(); nodeValue = nodeValue.nextSibling() ) {
                    if ( nodeValue.isElement() ) {
                        QDomElement elmValue = nodeValue.toElement();
                        Q_ASSERT( elmValue.tagName() == QString::fromLatin1("value") );
                        QString value = elmValue.attribute( QString::fromLatin1("value") );
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
    QString text;
    if ( Options::instance()->showDate() )  {
        if ( info->startDate().isNull() ) {
            // Don't append anything
        }
        else if ( info->endDate().isNull() )
            text += info->startDate();
        else
            text += info->startDate() + i18n(" to ") + info->endDate();

        if ( !text.isEmpty() ) {
            text = i18n("<b>Date:</b> ") + text + QString::fromLatin1("<br>");
        }
    }

    // PENDING(blackie) The key is used both as a key and a label, which is a problem here.
    if ( Options::instance()->showLocation() )  {
        QString location = info->optionValue( QString::fromLatin1("Locations") ).join( QString::fromLatin1(", ") );
        if ( !location.isEmpty() )
            text += i18n("<b>Location:</b> ") + location + QString::fromLatin1("<br>");
    }

    if ( Options::instance()->showNames() ) {
        QString persons = info->optionValue( QString::fromLatin1("Persons") ).join( QString::fromLatin1(", ") );
        if ( !persons.isEmpty() )
            text += i18n("<b>Persons:</b> ") + persons + QString::fromLatin1("<br>");
    }

    if ( Options::instance()->showDescription() && !info->description().isEmpty())  {
        if ( !text.isEmpty() )
            text += i18n("<b>Description:</b> ") +  info->description() + QString::fromLatin1("<br>");
    }

    if ( Options::instance()->showKeyWords() )  {
        QString keyWords = info->optionValue( QString::fromLatin1("Keywords") ).join( QString::fromLatin1(", ") );
        if ( !keyWords.isEmpty() )
            text += i18n("<b>Keywords:</b> ") + keyWords + QString::fromLatin1("<br>");
    }
    return text;
}
