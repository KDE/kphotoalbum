/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef RANGEWIDGET_H
#define RANGEWIDGET_H

#include "SearchInfo.h"

#include <QList>
#include <qobject.h>
class QGridLayout;
class QComboBox;

namespace Exif
{

class RangeWidget : public QObject
{
    Q_OBJECT

public:
    class Value
    {
    public:
        Value() {}
        Value(double value, const QString &text)
            : value(value)
            , text(text)
        {
        }
        double value;
        QString text;
    };

    typedef QList<Value> ValueList;

    RangeWidget(const QString &text, const QString &searchTag, const ValueList &list, QGridLayout *layout, int row);
    Exif::SearchInfo::Range range() const;

protected slots:
    void slotUpdateTo(int index);

protected:
    QString tagToLabel(const QString &tag);

private:
    QString m_searchTag;
    QComboBox *m_from;
    QComboBox *m_to;
    ValueList m_list;
};

}

#endif /* RANGEWIDGET_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
