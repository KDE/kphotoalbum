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
                    BlockFromDatabase };

class DeleteFiles : public QObject
{
    Q_OBJECT

public:
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
