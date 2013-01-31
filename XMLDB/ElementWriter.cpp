#include "ElementWriter.h"
#include <QXmlStreamWriter>

ElementWriter::ElementWriter(QXmlStreamWriter &writer, const QString &elementName, bool writeAtOnce)
    :m_writer(writer), m_elementName(elementName), m_haveWrittenStartTag(writeAtOnce)
{
    if (writeAtOnce)
        m_writer.writeStartElement(m_elementName);
}

void ElementWriter::writeStartElement()
{
    if ( m_haveWrittenStartTag )
        return;

    m_haveWrittenStartTag = true;
    m_writer.writeStartElement(m_elementName);
}


ElementWriter::~ElementWriter()
{
    if ( m_haveWrittenStartTag)
        m_writer.writeEndElement();
}
