// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef RANGEWIDGET_H
#define RANGEWIDGET_H

#include <kpaexif/SearchInfo.h>

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
        Value() { }
        Value(double value, const QString &text)
            : value(value)
            , text(text)
        {
        }
        double value;
        QString text;
    };

    typedef QList<Value> ValueList;

    RangeWidget(const QString &text, const QString &searchTag, const ValueList &list, QGridLayout *layout, int row, QObject *parent);
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
