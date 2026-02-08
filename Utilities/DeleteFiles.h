// SPDX-FileCopyrightText: 2012-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef UTILITIES_DELETEFILES_H
#define UTILITIES_DELETEFILES_H

#include <kpabase/FileNameList.h>

#include <QObject>

class KJob;

namespace Utilities
{

enum DeleteMethod { DeleteFromDisk,
                    MoveToTrash,
                    RemoveFromDatabase };

class DeleteFiles : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief deleteFiles deletes the given files and removes them from the database.
     * For any files in the database that currently exist on disk, \c method is applied.
     * When files are only removed from the database (\c DeleteMethod::RemoveFromDatabase),
     * they are put on a block list to prevent them from being re-added on the next scan for new images.
     * For any files in the database that currently do not exist on disk, the files are always just removed from the database.
     *
     * @param files the files to remove
     * @param method describes what should be done with (existing) files.
     * @return \c true if any file was actually removed, \c false otherwise.
     */
    static bool deleteFiles(const DB::FileNameList &files, DeleteMethod method);

private Q_SLOTS:
    void slotKIOJobCompleted(KJob *);

private:
    static DeleteFiles *s_instance;
    bool deleteFilesPrivate(const DB::FileNameList &files, DeleteMethod method);
};

} // namespace Utilities

#endif // UTILITIES_DELETEFILES_H
// vi:expandtab:tabstop=4 shiftwidth=4:
