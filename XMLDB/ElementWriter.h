#ifndef ELEMENTWRITER_H
#define ELEMENTWRITER_H

class QXmlStreamWriter;
#include <QString>

class ElementWriter
{
public:
    ElementWriter(QXmlStreamWriter& writer, const QString& elementName, bool writeAtOnce = true );
    void writeStartElement();
    ~ElementWriter();

private:
    QXmlStreamWriter& m_writer;
    QString m_elementName;
    bool m_haveWrittenStartTag;
};

#endif // ELEMENTWRITER_H
