#ifndef XMLREADER_H
#define XMLREADER_H

#include <QXmlStreamReader>

namespace XMLDB {

class XmlReader : public QXmlStreamReader
{
public:
    explicit XmlReader();

    QString attribute( const char* name, const QString& defaultValue = QString() );
    QString readNextStartElement( const char* expected = 0);
    bool readNextStartOrStopElement( const char* expectedStart, const char* expectedEnd);
    void readEndElement(const char* expected );
    bool hasAttribute(const char* name);
private:
    void reportError(const QString&);
    QString tokenToString(TokenType);
    TokenType readNextInternal();
};

}

#endif // XMLREADER_H
