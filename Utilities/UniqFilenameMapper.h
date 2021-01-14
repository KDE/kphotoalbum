/* SPDX-FileCopyrightText: 2008-2010 Henner Zeller <h.zeller@acm.org>

   based on Utilities::createUniqNameMap() by <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef UTILITIES_UNIQ_FILENAME_MAPPER_H
#define UTILITIES_UNIQ_FILENAME_MAPPER_H

#include <kpabase/FileName.h>

#include <QMap>
#include <QSet>
#include <QString>

namespace Utilities
{

/**
 * The UniqFilenameMapper creates flat filenames from arbitrary input filenames
 * so that there are no conflicts if they're written in one directory together.
 * The resulting names do not contain any path unless a targetDirectory is
 * given in the constructor.
 *
 * Example:
 * uniqNameFor("cd1/def.jpg")      -> def.jpg
 * uniqNameFor("cd1/abc/file.jpg") -> file.jpg
 * uniqNameFor("cd3/file.jpg")     -> file-1.jpg
 * uniqNameFor("cd1/abc/file.jpg") -> file.jpg    // file from above.
 */
class UniqFilenameMapper
{
public:
    UniqFilenameMapper();

    // Create a UniqFilenameMapper that returns filenames with the
    // targetDirectory prepended.
    // The UniqFilenameMapper makes sure, that generated filenames do not
    // previously exist in the targetDirectory.
    explicit UniqFilenameMapper(const QString &targetDirectory);

    // Create a unique, flat filename for the target directory. If this method
    // has been called before with the same argument, the unique name that has
    // been created before is returned (see example above).
    QString uniqNameFor(const DB::FileName &filename);

    // Reset all mappings.
    void reset();

private:
    UniqFilenameMapper(const UniqFilenameMapper &); // don't copy.

    bool fileClashes(const QString &file);

    const QString m_targetDirectory;
    typedef QMap<DB::FileName, QString> FileNameMap;
    FileNameMap m_origToUniq;
    QSet<QString> m_uniqFiles;
};
}
#endif /* UTILITIES_UNIQ_FILENAME_MAPPER_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
