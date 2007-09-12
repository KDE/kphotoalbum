/*
  Copyright (C) 2007 Tuomas Suutari <thsuut@utu.fi>
  Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/

#include "Set.h"

template <class T>
void Utilities::Set<T>::insert(const QValueList<T>& list)
{
    for (typename QValueList<T>::const_iterator i = list.begin();
         i != list.end(); ++i)
        insert(*i);
}

template <class T>
QValueList<T> Utilities::Set<T>::toList() const
{
    QValueList<T> l;
    for (typename Base::const_iterator i = Base::begin();
         i != Base::end(); ++i)
        l.push_back(*i);
    return l;
}

template <class T>
Utilities::Set<T>::Set<T>& Utilities::Set<T>::operator+=(const Set<T>& other)
{
    for (typename Set<T>::const_iterator i = other.begin();
         i != other.end(); ++i)
        insert(*i);
    return *this;
}

template <class T>
Utilities::Set<T>::Set<T>& Utilities::Set<T>::operator-=(const Set<T>& other)
{
    for (typename Set<T>::const_iterator i = other.begin();
         i != other.end(); ++i)
        erase(*i);
    return *this;
}

template <class T>
QDataStream& operator<<(QDataStream& stream, const Utilities::Set<T>& data)
{
    stream << static_cast<Q_INT16>(data.size());
    for (typename Utilities::Set<T>::const_iterator i = data.begin();
         i != data.end(); ++i)
        stream << *i;
    return stream;
}

template <class T>
QDataStream& operator>>(QDataStream& stream, Utilities::Set<T>& data)
{
    Q_INT16 count;
    stream >> count;
    for (int i = 0; i < count; ++i) {
        T item;
        stream >> item;
        data.insert(item);
    }
    return stream;
}

#define INSTANTIATE_SET_CLASS(T) \
template \
class Utilities::Set<T>

#define INSTANTIATE_SET_CLASS_WITH_OPERATORS(T) \
template \
class Utilities::Set<T>; \
\
template \
QDataStream& operator<<(QDataStream& stream, const Utilities::Set<T>& data); \
\
template \
QDataStream& operator>>(QDataStream& stream, Utilities::Set<T>& data)


INSTANTIATE_SET_CLASS(QString);

INSTANTIATE_SET_CLASS(int);

#include <qpair.h>
typedef QPair<QString, int> StringIntPair;
INSTANTIATE_SET_CLASS(StringIntPair);
typedef QPair<QString, QString> StringStringPair;
INSTANTIATE_SET_CLASS(StringStringPair);

#include "CategoryListView/DragItemInfo.h"
INSTANTIATE_SET_CLASS_WITH_OPERATORS(CategoryListView::DragItemInfo);

#include "ImageManager/RequestQueue.h"
INSTANTIATE_SET_CLASS(ImageManager::RequestQueue::ImageRequestReference);

#include "ImageManager/ImageRequest.h"
INSTANTIATE_SET_CLASS(ImageManager::ImageRequest*);
