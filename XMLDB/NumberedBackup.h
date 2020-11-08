/* SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NUMBEREDBACKUP_H
#define NUMBEREDBACKUP_H
#include <QStringList>

namespace DB
{
class UIDelegate;
}

namespace XMLDB
{
class NumberedBackup
{
public:
    explicit NumberedBackup(DB::UIDelegate &ui);
    void makeNumberedBackup();

protected:
    int getMaxId() const;
    QStringList backupFiles() const;
    int idForFile(const QString &fileName, bool &OK) const;
    void deleteOldBackupFiles();

private:
    DB::UIDelegate &m_ui;
};
}

#endif /* NUMBEREDBACKUP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
