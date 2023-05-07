// SPDX-FileCopyrightText: 2012-2013 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef ELEMENTWRITER_H
#define ELEMENTWRITER_H

class QXmlStreamWriter;
#include <QString>

namespace DB
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
