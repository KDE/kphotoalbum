#include "ImageNameStore.h"
#include "DB/ImageDB.h"

namespace RemoteControl {
ImageNameStore::ImageNameStore()
{
    // To avoid delays when the user shows all images the first time, lets pull all images now.
    for (const DB::FileName& fileName : DB::ImageDB::instance()->images()) {
        m_lastId++;
        m_idToNameMap.insert(m_lastId,fileName);
        m_nameToIdMap.insert(fileName,m_lastId);
    }
}

DB::FileName ImageNameStore::operator[](int id)
{
    return m_idToNameMap[id];
}

int ImageNameStore::operator[](const DB::FileName& fileName)
{
    auto iterator = m_nameToIdMap.find(fileName);
    if (iterator == m_nameToIdMap.end()) {
        m_lastId++;
        m_nameToIdMap.insert(fileName,m_lastId);
        m_idToNameMap.insert(m_lastId, fileName);
        return m_lastId;
    }
        return *iterator;
}

} // namespace RemoteControl
