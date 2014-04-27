#include "DiscoveryModel.h"
#include "Types.h"

namespace RemoteControl {

DiscoveryModel::DiscoveryModel(QObject* parent)
    : ThumbnailModel(parent)
{
}

int DiscoveryModel::count() const
{
    return m_count;
}

void DiscoveryModel::setImages(const QList<int>& images)
{
    m_allImages = images;
    resetImages();
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
    result = result.mid(0,m_count);
    result.push_front(DISCOVERYID);
    ThumbnailModel::setImages(result);

}

} // namespace RemoteControl
