/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ElementWriter.h"

#include <QXmlStreamWriter>

XMLDB::ElementWriter::ElementWriter(QXmlStreamWriter &writer, const QString &elementName, bool writeAtOnce)
    : m_writer(writer)
    , m_elementName(elementName)
    , m_haveWrittenStartTag(writeAtOnce)
{
    if (writeAtOnce)
        m_writer.writeStartElement(m_elementName);
}

void XMLDB::ElementWriter::writeStartElement()
{
    if (m_haveWrittenStartTag)
        return;

    m_haveWrittenStartTag = true;
    m_writer.writeStartElement(m_elementName);
}

XMLDB::ElementWriter::~ElementWriter()
{
    if (m_haveWrittenStartTag)
        m_writer.writeEndElement();
}
// vi:expandtab:tabstop=4 shiftwidth=4:
