/* Copyright (C) 2013-2019 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "XmlReader.h"

#include <DB/UIDelegate.h>

#include <KLocalizedString>

namespace XMLDB
{

XmlReader::XmlReader(DB::UIDelegate &ui, const QString &friendlyStreamName)
    : m_ui(ui)
    , m_streamName(friendlyStreamName)
{
}

QString XmlReader::attribute(const QString &name, const QString &defaultValue)
{
    QStringRef ref = attributes().value(name);
    if (ref.isNull())
        return defaultValue;
    else
        return ref.toString();
}

ElementInfo XmlReader::readNextStartOrStopElement(const QString &expectedStart)
{
    if (m_peek.isValid) {
        m_peek.isValid = false;
        return m_peek;
    }

    TokenType type = readNextInternal();

    if (hasError())
        reportError(i18n("Error reading next element"));

    if (type != StartElement && type != EndElement)
        reportError(i18n("Expected to read a start or stop element, but read %1", tokenString()));

    const QString elementName = name().toString();
    if (type == StartElement) {
        if (!expectedStart.isNull() && elementName != expectedStart)
            reportError(i18n("Expected to read %1, but read %2", expectedStart, elementName));
    }

    return ElementInfo(type == StartElement, elementName);
}

void XmlReader::readEndElement(bool readNextElement)
{
    if (readNextElement)
        readNextInternal();
    if (tokenType() != EndElement)
        reportError(i18n("Expected to read an end element but read %1", tokenString()));
}

bool XmlReader::hasAttribute(const QString &name)
{
    return attributes().hasAttribute(name);
}

ElementInfo XmlReader::peekNext()
{
    if (m_peek.isValid)
        return m_peek;
    m_peek = readNextStartOrStopElement(QString());
    return m_peek;
}

void XmlReader::complainStartElementExpected(const QString &name)
{
    reportError(i18n("Expected to read start element '%1'", name));
}

void XmlReader::reportError(const QString &text)
{
    QString message = i18n(
        "<p>An error was encountered on line %1, column %2:<nl/>"
        "<message>%3</message></p>",
        lineNumber(), columnNumber(), text);
    if (hasError())
        message += i18n("<p>Additional error information:<nl/><message>%1</message></p>", errorString());
    message += xi18n("<p>Database path: <filename>%1</filename></p>", m_streamName);

    m_ui.error(QStringLiteral("XmlReader: error in line %1, column %2 (%3)")
                   .arg(lineNumber())
                   .arg(columnNumber())
                   .arg(errorString()),
               message, i18n("Error while reading database file"));
    exit(-1);
}

QXmlStreamReader::TokenType XmlReader::readNextInternal()
{
    forever
    {
        TokenType type = readNext();
        if (type == Comment || type == StartDocument)
            continue;
        else if (type == Characters) {
            if (isWhitespace())
                continue;
        } else
            return type;
    }
}

}

// vi:expandtab:tabstop=4 shiftwidth=4:
