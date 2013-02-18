#ifndef XMLREADER_H
#define XMLREADER_H

#include <QXmlStreamReader>
#include <QSharedPointer>

namespace XMLDB {

class XmlReader : public QXmlStreamReader
{
public:
    enum StartElementRequirement {MustFindStartElement, StartElementIsOptional};
    explicit XmlReader();

    QString attribute(const QString &name, const QString& defaultValue = QString() );
    bool readNextStartElement(const QString &expected, StartElementRequirement );
    bool readNextStartOrStopElement(const QString &expectedStart);
    void readEndElement();
    bool hasAttribute(const QString& name);
private:
    void reportError(const QString&);
    QString tokenToString(TokenType);
    TokenType readNextInternal();
};

typedef QSharedPointer<XmlReader> ReaderPtr;

}

#endif // XMLREADER_H
