/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "FileNameList.h"

DB::FileNameList::FileNameList(const QList<DB::FileName> &other)
{
    QList<DB::FileName>::operator=(other);
}

DB::FileNameList::FileNameList(const QStringList &files)
{
    for (const QString &file : files)
        append(DB::FileName::fromAbsolutePath(file));
}

QStringList DB::FileNameList::toStringList(DB::PathType type) const
{
    QStringList res;

    for (const DB::FileName &fileName : *this) {
        if (type == DB::RelativeToImageRoot)
            res.append(fileName.relative());
        else
            res.append(fileName.absolute());
    }
    return res;
}

DB::FileNameList &DB::FileNameList::operator<<(const DB::FileName &fileName)
{
    QList<DB::FileName>::operator<<(fileName);
    return *this;
}

DB::FileNameList DB::FileNameList::reversed() const
{
    FileNameList res;
    for (const FileName &fileName : *this) {
        res.prepend(fileName);
    }
    return res;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
