#include "SearchInfo.h"

namespace RemoteControl {


void RemoteControl::SearchInfo::addCategory(const QString& category)
{
    m_categories.append(category);
}

void SearchInfo::addValue(const QString& value)
{
    m_values.append(value);
}

void SearchInfo::pop()
{
    m_categories.pop();
    if (m_values.count() > m_categories.count())
        m_values.pop();
}

void SearchInfo::clear()
{
    m_categories.clear();
    m_values.clear();
}

QDataStream&operator<<(QDataStream& stream, const SearchInfo& searchInfo)
{
    stream << searchInfo.m_categories << searchInfo.m_values;
    return stream;
}

QDataStream&operator>>(QDataStream& stream, SearchInfo& searchInfo)
{
    stream >> searchInfo.m_categories >> searchInfo.m_values;
    return stream;
}

} // namespace RemoteControl
