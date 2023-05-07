// SPDX-FileCopyrightText: 2005-2006, 2010 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef NUMBEREDBACKUP_H
#define NUMBEREDBACKUP_H
#include <QStringList>

namespace DB
{
class UIDelegate;

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
