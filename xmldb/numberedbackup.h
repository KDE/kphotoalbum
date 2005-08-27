#ifndef NUMBEREDBACKUP_H
#define NUMBEREDBACKUP_H
#include <qstringlist.h>

namespace XMLDB {
    class NumberedBackup
    {
    public:
        void makeNumberedBackup();
    protected:
        int getMaxId() const;
        QStringList backupFiles() const;
        int idForFile( const QString& fileName, bool& OK ) const;
        void deleteOldBackupFiles();
    };
}

#endif /* NUMBEREDBACKUP_H */

