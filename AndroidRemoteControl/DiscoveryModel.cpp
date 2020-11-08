/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

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

#include "DiscoveryModel.h"
#include "Action.h"
#include "Types.h"

namespace RemoteControl
{

DiscoveryModel::DiscoveryModel(QObject *parent)
    : ThumbnailModel(parent)
{
}

int DiscoveryModel::count() const
{
    return m_count;
}

void DiscoveryModel::setImages(const QList<int> &images)
{
    m_allImages = images;
    resetImages();
}

void DiscoveryModel::setCurrentSelection(const QList<int> &selection, const QList<int> &allImages)
{
    ThumbnailModel::setImages(selection);
    m_allImages = allImages;
}

void DiscoveryModel::setCurrentAction(DiscoverAction *action)
{
    m_action = action;
}

void DiscoveryModel::setCount(int count)
{
    if (m_count != count) {
        m_count = count;
        emit countChanged();
        resetImages();
    }
}

void DiscoveryModel::resetImages()
{
    if (m_count == 0 || m_allImages.isEmpty())
        return;

    QList<int> result = m_allImages;
    std::random_shuffle(result.begin(), result.end());
    result = result.mid(0, m_count);
    result.push_front(DISCOVERYID);
    ThumbnailModel::setImages(result);
    m_action->setCurrentSelection(result, m_allImages);
}

} // namespace RemoteControl
