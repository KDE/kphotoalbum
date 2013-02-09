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
};

}

#endif // XMLREADER_H
