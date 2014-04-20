#include "ImageDetails.h"

namespace RemoteControl {

ImageDetails&ImageDetails::instance()
{
    static ImageDetails instance;
    return instance;
}

QStringList ImageDetails::categories() const
{
    return m_categories.keys();
}

QStringList ImageDetails::itemsOfCategory(const QString category)
{
    return m_categories[category];
}

void ImageDetails::clear()
{
    m_fileName.clear();
    m_date.clear();
    m_description.clear();
    m_categories.clear();
    emit updated();
}

void ImageDetails::setData(const ImageDetailsCommand& data)
{
    m_fileName = data.fileName;
    m_date = data.date;
    m_description = data.description;
    m_categories = data.categories;
    emit updated();
}

} // namespace RemoteControl
