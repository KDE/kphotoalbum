// SPDX-FileCopyrightText: 2013 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2014-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef XMLREADER_H
#define XMLREADER_H

#include <QSharedPointer>
#include <QXmlStreamReader>

namespace DB
{
class UIDelegate;

struct ElementInfo {
    ElementInfo(bool isStartToken, const QString &tokenName)
        : isValid(true)
        , isStartToken(isStartToken)
        , tokenName(tokenName)
    {
    }
    ElementInfo()
        : isValid(false)
        , isStartToken(false)
    {
    }

    bool isValid;
    bool isStartToken;
    QString tokenName;
};

class XmlReader : public QXmlStreamReader
{
public:
    /**
     * @brief XmlReader
     * @param ui the UIDelegate for error messages
     * @param friendlyStreamName a stream/file name to be displayed in messages
     */
    explicit XmlReader(DB::UIDelegate &ui, const QString &friendlyStreamName);

    QString attribute(const QString &name, const QString &defaultValue = QString());
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
    bool hasAttribute(const QString &name);
    ElementInfo peekNext();
    [[noreturn]] void complainStartElementExpected(const QString &name);
    void setFileVersion(int version);
    int fileVersion() const;

private:
    [[noreturn]] void reportError(const QString &);
    QString tokenToString(TokenType);
    TokenType readNextInternal();

    DB::UIDelegate &m_ui;
    ElementInfo m_peek;
    const QString m_streamName;
    int m_fileVersion;
};

typedef QSharedPointer<XmlReader> ReaderPtr;

}

#endif // XMLREADER_H
// vi:expandtab:tabstop=4 shiftwidth=4:
