/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ELEMENTWRITER_H
#define ELEMENTWRITER_H

class QXmlStreamWriter;
#include <QString>

namespace XMLDB
{
class ElementWriter
{
public:
    ElementWriter(QXmlStreamWriter &writer, const QString &elementName, bool writeAtOnce = true);
    void writeStartElement();
    ~ElementWriter();

private:
    QXmlStreamWriter &m_writer;
    QString m_elementName;
    bool m_haveWrittenStartTag;
};
}

#endif // ELEMENTWRITER_H
// vi:expandtab:tabstop=4 shiftwidth=4:
