#include "ImageNameStore.h"
#include "DB/ImageDB.h"

namespace RemoteControl {

ImageNameStore::ImageNameStore()
{
    int id = 0;
    for (const DB::FileName& fileName : DB::ImageDB::instance()->images()) {
        id++;
        m_idToNameMap.insert(id,fileName);
        m_nameToIdMap.insert(fileName,id);
    }
}

DB::FileName ImageNameStore::operator[](int id) const
{
    return m_idToNameMap[id];
}

int ImageNameStore::operator[](const DB::FileName& fileName) const
{
    return m_nameToIdMap[fileName];
}

} // namespace RemoteControl
