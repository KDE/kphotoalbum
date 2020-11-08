/* SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef IMAGEINFOLIST_H
#define IMAGEINFOLIST_H
#include "ImageInfo.h"
#include "ImageInfoPtr.h"

#include <QList>

namespace DB
{
class FileNameList;

class ImageInfoList : public QList<ImageInfoPtr>
{
public:
    void sortAndMergeBackIn(ImageInfoList &subListToSort);
    ImageInfoList sort() const;
    void appendList(ImageInfoList &other);
    void printItems();
    bool isSorted();
    void mergeIn(ImageInfoList list);
    void remove(const ImageInfoPtr &info);
    DB::FileNameList files() const;
};

typedef QList<ImageInfoPtr>::Iterator ImageInfoListIterator;
typedef QList<ImageInfoPtr>::ConstIterator ImageInfoListConstIterator;

}

#endif /* IMAGEINFOLIST_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
