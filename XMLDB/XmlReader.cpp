#include "XmlReader.h"

namespace XMLDB {

XmlReader::XmlReader()
{
}

QString XmlReader::attribute( const char* name, const QString& defaultValue )
{
    const QString attributeName = QString::fromLatin1(name);
    if ( attributes().hasAttribute(attributeName ))
        return attributes().value(attributeName).toString();
    else
        return defaultValue;
}

QString XmlReader::readNextStartElement( const char* expected )
{
    const bool ok = QXmlStreamReader::readNextStartElement();
    if ( !ok ) {
        qFatal("Error reading next element. Error was:\n%s", qPrintable(errorString()));
        exit(-1);
    }

    const QString elementName = name().toString();
    if ( expected ) {
        if ( elementName != QString::fromUtf8(expected)) {
            qFatal("Expected to read %s, but read %s", expected, qPrintable(elementName));
            exit(-1);
        }
    }

    return elementName;
}


}
