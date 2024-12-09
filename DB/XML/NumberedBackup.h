// SPDX-FileCopyrightText: 2005-2006, 2010 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef NUMBEREDBACKUP_H
#define NUMBEREDBACKUP_H

#include <QFileInfo>
#include <QStringList>

namespace DB
{
class UIDelegate;

/**
 * This class manages the creation and removal of numbered backup files.  The
 * numbered backup files are created in the same directory as the XML database
 * file, and each backup filename is derived from the XML database filename
 * with an embedded sequential number.
 *
 * The number of backup files to keep is configurable.  Each backup file can
 * optionally be compressed.  These options are configured using the
 * KPhotoAlbum configuration file.
 */
class NumberedBackup
{
public:
    explicit NumberedBackup(DB::UIDelegate &ui, const QString &xmlFileName);

    /**
     * Attempts to create a numbered backup file of the XML database file
     * passed to the constructor.  Automatically removes older numbered backup
     * files (if configured) and compresses the new backup file (if
     * configured).
     *
     * Any errors are reported to the UIDelegate passed to the constructor.
     */
    void makeNumberedBackup();

protected:
    /**
     * Gets the maximum ID number of any existing numbered backup files in the
     * same directory as the XML database file passed to the constructor.
     *
     * @return the numeric backup ID or zero if no numbered backup files exist
     */
    int getMaxId() const;

    /**
     * @return the list of any numbered backup filenames in the same directory
     * as the XML database file (the list is empty if no numbered backup files
     * exist).
     */
    QStringList backupFiles() const;

    /**
     * Attempts to extract the numeric backup file ID from a filename.
     *
     * @return the numeric ID (if OK is true) or -1 (if OK is false) if the
     * supplied filename is invalid
     */
    int idForFile(const QString &fileName, bool &OK) const;

    /**
     * Deletes older numbered backup files in the same directory as the XML
     * database file (if there are more numbered backup files than the
     * configured limit).
     */
    void deleteOldBackupFiles();

private:
    /**
     * Observer for error handling.
     */
    DB::UIDelegate &m_ui;

    /**
     * Info for the XML database file.
     */
    QFileInfo m_xmlFileInfo;
};
}

#endif /* NUMBEREDBACKUP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
