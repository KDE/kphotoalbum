/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include "RangeWidget.h"

#include <KComboBox>
#include <QGridLayout>
#include <qlabel.h>

Exif::RangeWidget::RangeWidget(const QString &text, const QString &searchTag, const ValueList &list, QGridLayout *layout, int row)
    : QObject(layout->widget())
    , m_searchTag(searchTag)
    , m_list(list)
{
    int col = 0;

    // widget layout: <title text> <from_value> "to" <to_value>
    // register title text:
    QLabel *label = new QLabel(text);
    layout->addWidget(label, row, col++);

    // register from-field:
    m_from = new KComboBox;
    layout->addWidget(m_from, row, col++);

    // register filler between from- and to-field:
    label = new QLabel(QString::fromLatin1("to"));
    layout->addWidget(label, row, col++);

    // register to-field:
    m_to = new KComboBox;
    layout->addWidget(m_to, row, col++);

    Q_ASSERT(list.count() > 2);
    ValueList::ConstIterator it = list.begin();
    m_from->addItem(QString::fromLatin1("< %1").arg((*it).text));

    for (; it != list.end(); ++it) {
        m_from->addItem((*it).text);
    }

    m_from->addItem(QString::fromLatin1("> %1").arg(list.last().text));
    slotUpdateTo(0);
    m_to->setCurrentIndex(m_to->count() - 1); // set range to be min->max

    connect(m_from, SIGNAL(activated(int)), this, SLOT(slotUpdateTo(int)));
}

void Exif::RangeWidget::slotUpdateTo(int fromIndex)
{
    m_to->clear();

    if (fromIndex == 0)
        m_to->addItem(QString::fromLatin1("< %1").arg(m_list.first().text));
    else
        fromIndex--;

    for (int i = fromIndex; i < m_list.count(); ++i) {
        m_to->addItem(m_list[i].text);
    }
    m_to->addItem(QString::fromLatin1("> %1").arg(m_list.last().text));
}

Exif::SearchInfo::Range Exif::RangeWidget::range() const
{
    SearchInfo::Range result(m_searchTag);
    result.min = m_list.first().value;
    result.max = m_list.last().value;
    if (m_from->currentIndex() == 0)
        result.isLowerMin = true;
    else if (m_from->currentIndex() == m_from->count() - 1)
        result.isLowerMax = true;
    else
        result.min = m_list[m_from->currentIndex() - 1].value;

    if (m_to->currentIndex() == 0 && m_from->currentIndex() == 0)
        result.isUpperMin = true;
    else if (m_to->currentIndex() == m_to->count() - 1)
        result.isUpperMax = true;
    else
        result.max = m_list[m_to->currentIndex() + m_from->currentIndex() - 1].value;

    return result;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
