/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SEARCHDIALOGSETTINGS_H
#define SEARCHDIALOGSETTINGS_H

#include <QList>
#include <qcheckbox.h>

namespace Exif
{

template <class T>
class Setting
{
public:
    Setting() { }
    Setting(QCheckBox *cb, T value)
        : cb(cb)
        , value(value)
    {
    }
    QCheckBox *cb;
    T value;
};

template <class T>
class Settings : public QList<Setting<T>>
{
public:
    QList<T> selected()
    {
        QList<T> result;
        for (typename QList<Setting<T>>::Iterator it = this->begin(); it != this->end(); ++it) {
            if ((*it).cb->isChecked())
                result.append((*it).value);
        }
        return result;
    }
};

}

#endif /* SEARCHDIALOGSETTINGS_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
