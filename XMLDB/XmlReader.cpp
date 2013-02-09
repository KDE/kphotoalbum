#include "XmlReader.h"
#include <KLocale>

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
    if ( !ok )
        reportError(i18n("Error reading next element"));

    const QString elementName = name().toString();
    if ( expected ) {
        if ( elementName != QString::fromUtf8(expected))
            reportError(i18n("Expected to read %1, but read %2").arg(QString::fromUtf8(expected)).arg(elementName));
    }

    return elementName;
}

bool XmlReader::readNextStartOrStopElement(const char *expectedStart, const char* expectedEnd)
{
    TokenType type = readNextInternal();

    if ( hasError() )
        reportError(i18n("Error reading next element"));

    if ( type != StartElement && type != EndElement )
        reportError(i18n("Expected to read a start or stop element, but read %1").arg(tokenToString(type)));

    const QString elementName = name().toString();
    if ( type == StartElement ) {
        if ( elementName != QString::fromUtf8(expectedStart))
            reportError(i18n("Expected to read %1, but read %2").arg(QString::fromUtf8(expectedStart)).arg(elementName));
    }
    else {
        if ( elementName != QString::fromUtf8(expectedEnd))
            reportError(i18n("Expected to read /%1, but read /%2").arg(QString::fromUtf8(expectedEnd)).arg(elementName));
    }
    return (type == StartElement);
}

void XmlReader::readEndElement(const char *expected)
{
    TokenType type = readNextInternal();
    if ( type != EndElement )
        reportError(i18n("Expected to read an end element (named %1) but read an %2").arg(QString::fromUtf8(expected)).arg(tokenToString(type)));

    const QString elementName = name().toString();
    if ( elementName != QString::fromUtf8(expected))
        reportError(i18n("Expected to read %1, but read %2").arg(QString::fromUtf8(expected)).arg(elementName));
}

bool XmlReader::hasAttribute(const char *name)
{
    return attributes().hasAttribute(QString::fromUtf8(name));
}

void XmlReader::reportError(const QString & text)
{
    QString message = i18n("On line %1, column %2:\n%3").arg(lineNumber()).arg(columnNumber()).arg(text);
    if ( hasError() )
        message += QString::fromUtf8("\n") + errorString();

    qFatal("%s", qPrintable(message));
}

QString XmlReader::tokenToString(QXmlStreamReader::TokenType type)
{
    switch ( type ) {
    case NoToken: return QString::fromUtf8("NoToken");
    case Invalid: return QString::fromUtf8("Invalid");
    case StartDocument: return QString::fromUtf8("StartDocument");
    case EndDocument: return QString::fromUtf8("EndDocument");
    case StartElement: return QString::fromUtf8("StartElement");
    case EndElement: return QString::fromUtf8("EndElement");
    case Characters: return QString::fromUtf8("Characters");
    case Comment: return QString::fromUtf8("Comment");
    case DTD: return QString::fromUtf8("DTD");
    case EntityReference: return QString::fromUtf8("EntityReference");
    case ProcessingInstruction: return QString::fromUtf8("ProcessingInstruction");

    }
    return QString();
}

QXmlStreamReader::TokenType XmlReader::readNextInternal()
{
    forever {
        TokenType type = readNext();
        if ( type == Comment )
            continue;
        else if (type == Characters ) {
            if (text().toString().simplified().isEmpty())
            continue;
        }
        else
            return type;
    }
}

}

