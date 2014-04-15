#ifndef REMOTECONTROL_IMAGENAMESTORE_H
#define REMOTECONTROL_IMAGENAMESTORE_H

#include "DB/FileName.h"
#include <QHash>

namespace RemoteControl {

class ImageNameStore
{
public:
    ImageNameStore();
    DB::FileName operator[](int id);
    int operator[](const DB::FileName& fileName);

private:
    QHash<int,DB::FileName> m_idToNameMap;
    QHash<DB::FileName,int> m_nameToIdMap;
    int m_lastId = 0;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_IMAGENAMESTORE_H
