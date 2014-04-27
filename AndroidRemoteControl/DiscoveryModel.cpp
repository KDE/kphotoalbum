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
    QList<int> result;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 10000);
    result.append(DISCOVERYID);
    for (int i=0;i<m_count;++i)
        result.append(dis(gen));
    setImages(result);

}

} // namespace RemoteControl
