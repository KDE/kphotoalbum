/* Copyright (C) 2012 Jesper K. Pedersen <blackie@kde.org>

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
// vi:expandtab:tabstop=4 shiftwidth=4:
