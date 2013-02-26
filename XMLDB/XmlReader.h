#ifndef XMLREADER_H
#define XMLREADER_H

#include <QXmlStreamReader>
#include <QSharedPointer>

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
    explicit XmlReader();

    QString attribute(const QString &name, const QString& defaultValue = QString() );
    ElementInfo readNextStartOrStopElement(const QString &expectedStart);
    void readEndElement();
    bool hasAttribute(const QString& name);
    ElementInfo peekNext();
    void complainStartElementExpected(const QString& name);

private:
    void reportError(const QString&);
    QString tokenToString(TokenType);
    TokenType readNextInternal();

    ElementInfo m_peek;
};

typedef QSharedPointer<XmlReader> ReaderPtr;

}

#endif // XMLREADER_H
