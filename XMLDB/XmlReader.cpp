#include "XmlReader.h"
#include <KLocale>

namespace XMLDB {

XmlReader::XmlReader()
{
}

QString XmlReader::attribute( const QString& name, const QString& defaultValue )
{
    QStringRef ref = attributes().value(name);
    if ( ref.isNull() )
        return defaultValue;
    else
        return ref.toString();
}

bool XmlReader::readNextStartElement(const QString& expected , StartElementRequirement requirement)
{
    const bool ok = QXmlStreamReader::readNextStartElement();
    if ( !ok ) {
        if ( error() != NoError || requirement == MustFindStartElement )
            reportError(i18n("Error reading next element"));
        else
            return false;
    }

    const QString elementName = name().toString();
    if ( elementName != expected)
        reportError(i18n("Expected to read %1, but read %2").arg(expected).arg(elementName));

    return true;
}

bool XmlReader::readNextStartOrStopElement(const QString& expectedStart)
{
    TokenType type = readNextInternal();

    if ( hasError() )
        reportError(i18n("Error reading next element"));

    if ( type != StartElement && type != EndElement )
        reportError(i18n("Expected to read a start or stop element, but read %1").arg(tokenToString(type)));

    const QString elementName = name().toString();
    if ( type == StartElement ) {
        if ( elementName != expectedStart)
            reportError(i18n("Expected to read %1, but read %2").arg(expectedStart).arg(elementName));
    }
    return (type == StartElement);
}

void XmlReader::readEndElement()
{
    TokenType type = readNextInternal();
    if ( type != EndElement )
        reportError(i18n("Expected to read an end element but read an %2").arg(tokenToString(type)));
}

bool XmlReader::hasAttribute(const QString &name)
{
    return attributes().hasAttribute(name);
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
            if (isWhitespace())
                continue;
        }
        else
            return type;
    }
}

}

