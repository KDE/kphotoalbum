// SPDX-FileCopyrightText: 2012-2013 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ElementWriter.h"

#include <QXmlStreamWriter>

DB::ElementWriter::ElementWriter(QXmlStreamWriter &writer, const QString &elementName, bool writeAtOnce)
    : m_writer(writer)
    , m_elementName(elementName)
    , m_haveWrittenStartTag(writeAtOnce)
{
    if (writeAtOnce)
        m_writer.writeStartElement(m_elementName);
}

void DB::ElementWriter::writeStartElement()
{
    if (m_haveWrittenStartTag)
        return;

    m_haveWrittenStartTag = true;
    m_writer.writeStartElement(m_elementName);
}

DB::ElementWriter::~ElementWriter()
{
    if (m_haveWrittenStartTag)
        m_writer.writeEndElement();
}
// vi:expandtab:tabstop=4 shiftwidth=4:
