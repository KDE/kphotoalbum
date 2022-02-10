// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DiscoveryModel.h"
#include "../RemoteControl/Types.h"
#include "Action.h"
#include <random>

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
        Q_EMIT countChanged();
        resetImages();
    }
}

void DiscoveryModel::resetImages()
{
    if (m_count == 0 || m_allImages.isEmpty())
        return;

    QList<int> result = m_allImages;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(result.begin(), result.end(), g);
    result = result.mid(0, m_count);
    result.push_front(DISCOVERYID);
    ThumbnailModel::setImages(result);
    m_action->setCurrentSelection(result, m_allImages);
}

} // namespace RemoteControl

#include "moc_DiscoveryModel.cpp"
