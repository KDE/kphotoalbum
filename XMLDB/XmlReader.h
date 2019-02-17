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

#ifndef XMLREADER_H
#define XMLREADER_H

#include <QXmlStreamReader>
#include <QSharedPointer>

namespace DB {
class UIDelegate;
}

namespace XMLDB {

struct ElementInfo {
    ElementInfo(bool isStartToken, const QString& tokenName )
        : isValid(true), isStartToken(isStartToken),tokenName(tokenName) {}
    ElementInfo() : isValid(false) {}

    bool isValid;
    bool isStartToken;
    QString tokenName;
};

class XmlReader : public QXmlStreamReader
{
public:
    explicit XmlReader(DB::UIDelegate &ui);

    QString attribute(const QString &name, const QString& defaultValue = QString() );
    ElementInfo readNextStartOrStopElement(const QString &expectedStart);
    /**
     * Read the next element and ensure that it's an EndElement.
     * If the XmlReader has already read the EndElement (e.g. by calling readNextStartOrStopElement()),
     * but you want to use this method to ensure consistent error messages, you can
     * set the parameter readNextElement to false.
     *
     * @param readNextElement if set to false, don't read the next element.
     */
    void readEndElement(bool readNextElement = true);
    bool hasAttribute(const QString& name);
    ElementInfo peekNext();
    [[noreturn]] void complainStartElementExpected(const QString& name);

private:
    [[noreturn]] void reportError(const QString&);
    QString tokenToString(TokenType);
    TokenType readNextInternal();

    DB::UIDelegate &m_ui;
    ElementInfo m_peek;
};

typedef QSharedPointer<XmlReader> ReaderPtr;

}

#endif // XMLREADER_H
// vi:expandtab:tabstop=4 shiftwidth=4:
