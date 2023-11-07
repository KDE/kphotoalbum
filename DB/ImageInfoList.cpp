// SPDX-FileCopyrightText: 2005 Stephan Binner <binner@kde.org>
// SPDX-FileCopyrightText: 2005-2013 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007 Baptiste Mathus <ml@batmat.net>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2007-2008 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2008 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2008 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2014-2020 Robert Krawitz <rlk@alum.mit.edu>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImageInfoList.h"

#include "ImageInfo.h"

#include <Utilities/FastDateTime.h>
#include <kpabase/FileNameList.h>
#include <kpabase/Logging.h>

#include <KLocalizedString>
#include <QVector>
#include <QtAlgorithms>
using namespace DB;

class SortableImageInfo
{
public:
    SortableImageInfo(const Utilities::FastDateTime &datetime, const QString &string, const ImageInfoPtr &info)
        : m_dt(datetime)
        , m_st(string)
        , m_in(info)
    {
    }
    SortableImageInfo() = default;
    const Utilities::FastDateTime &DateTime(void) const { return m_dt; }
    const QString &String(void) const { return m_st; }
    const ImageInfoPtr &ImageInfo(void) const { return m_in; }
    bool operator==(const SortableImageInfo &other) const { return m_dt == other.m_dt && m_st == other.m_st; }
    bool operator!=(const SortableImageInfo &other) const { return m_dt != other.m_dt || m_st != other.m_st; }
    bool operator>(const SortableImageInfo &other) const
    {
        if (m_dt != other.m_dt) {
            return m_dt > other.m_dt;
        } else {
            return m_st > other.m_st;
        }
    }
    bool operator<(const SortableImageInfo &other) const
    {
        if (m_dt != other.m_dt) {
            return m_dt < other.m_dt;
        } else {
            return m_st < other.m_st;
        }
    }
    bool operator>=(const SortableImageInfo &other) const { return *this == other || *this > other; }
    bool operator<=(const SortableImageInfo &other) const { return *this == other || *this < other; }

private:
    Utilities::FastDateTime m_dt;
    QString m_st;
    ImageInfoPtr m_in;
};

ImageInfoList ImageInfoList::sort() const
{
    QVector<SortableImageInfo> vec;
    for (ImageInfoListConstIterator it = constBegin(); it != constEnd(); ++it) {
        vec.append(SortableImageInfo((*it)->date().start(), (*it)->fileName().absolute(), *it));
    }

    std::sort(vec.begin(), vec.end());

    ImageInfoList res;
    for (QVector<SortableImageInfo>::ConstIterator mapIt = vec.constBegin(); mapIt != vec.constEnd(); ++mapIt) {
        res.append(mapIt->ImageInfo());
    }
    return res;
}

void ImageInfoList::sortAndMergeBackIn(ImageInfoList &subListToSort)
{
    if (subListToSort.isEmpty()) {
        return;
    }
    ImageInfoList sorted = subListToSort.sort();

    // depending on view sort order, the first or the last index may be the first index
    const int insertIndex = qMin(indexOf(subListToSort.constFirst()), indexOf(subListToSort.constLast()));
    Q_ASSERT(insertIndex >= 0);

    // Delete the items we will merge in.
    for (ImageInfoListIterator it = sorted.begin(); it != sorted.end(); ++it)
        remove(*it);

    ImageInfoListIterator insertIt = begin() + insertIndex;

    // Now merge in the items
    for (ImageInfoListIterator it = sorted.begin(); it != sorted.end(); ++it) {
        insertIt = insert(insertIt, *it);
        ++insertIt;
    }
}

void ImageInfoList::appendList(ImageInfoList &list)
{
    for (ImageInfoListConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        append(*it);
    }
}

void ImageInfoList::printItems()
{
    for (ImageInfoListConstIterator it = constBegin(); it != constEnd(); ++it) {
        qCDebug(DBLog) << (*it)->fileName().absolute();
    }
}

bool ImageInfoList::isSorted()
{
    if (count() == 0)
        return true;

    Utilities::FastDateTime prev = first()->date().start();
    QString prevFile = first()->fileName().absolute();
    for (ImageInfoListConstIterator it = constBegin(); it != constEnd(); ++it) {
        Utilities::FastDateTime cur = (*it)->date().start();
        QString curFile = (*it)->fileName().absolute();
        if (prev > cur || (prev == cur && prevFile > curFile))
            return false;
        prev = cur;
        prevFile = curFile;
    }
    return true;
}

void ImageInfoList::mergeIn(ImageInfoList other)
{
    ImageInfoList tmp;

    for (ImageInfoListConstIterator it = constBegin(); it != constEnd(); ++it) {
        Utilities::FastDateTime thisDate = (*it)->date().start();
        QString thisFileName = (*it)->fileName().absolute();
        while (other.count() != 0) {
            Utilities::FastDateTime otherDate = other.first()->date().start();
            QString otherFileName = other.first()->fileName().absolute();
            if (otherDate < thisDate || (otherDate == thisDate && otherFileName < thisFileName)) {
                tmp.append(other[0]);
                other.pop_front();
            } else
                break;
        }
        tmp.append(*it);
    }
    tmp.appendList(other);
    *this = tmp;
}

void ImageInfoList::remove(const ImageInfoPtr &info)
{
    for (ImageInfoListIterator it = begin(); it != end(); ++it) {
        if ((*(*it)) == *info) {
            QList<ImageInfoPtr>::erase(it);
            return;
        }
    }
}

DB::FileNameList ImageInfoList::files(DB::MediaType type) const
{
    DB::FileNameList res;
    for (const ImageInfoPtr &info : *this) {
        if (info->mediaType() & type) {
            res.append(info->fileName());
        }
    }
    return res;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
