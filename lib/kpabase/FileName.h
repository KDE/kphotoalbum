// SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef KPABASE_FILENAME_H
#define KPABASE_FILENAME_H

#include <QSet>
#include <QString>
#include <QUrl>
#include <QtCore/qmetatype.h>

namespace DB
{

/**
 * @brief The FileName class represents a file inside the image root directory.
 * For performance reasons, the class stores both the relative and absolute file name.
 */
class FileName
{
public:
    /**
     * @brief FileName constructs a null FileName.
     */
    FileName();
    /**
     * @brief fromAbsolutePath creates a FileName from an absolute path within the image root.
     * \attention fromAbsolutePath depends on the DB::SettingsData singleton!
     * @param fileName the absolute path to a file inside the image root
     * @return a FileName for the given path, or a null FileName if the \p fileName was invalid or outside the image root
     */
    static FileName fromAbsolutePath(const QString &fileName);
    /**
     * @brief fromRelativePath creates a FileName from a from a path that is relative to the image root.
     * \attention fromAbsolutePath depends on the DB::SettingsData singleton!
     * @param fileName the relative (compared to the image root) path to a file
     * @return a FileName for the given path, or a null FileName if the \p fileName was invalid
     */
    static FileName fromRelativePath(const QString &fileName);
    /**
     * @brief absolute
     * @attention Do not call this function if isNull() is true!
     * @return the absolute path if the FileName is not null
     */
    QString absolute() const;
    /**
     * @brief relative
     * @attention Do not call this function if isNull() is true!
     * @return the relative path if the FileName is not null
     */
    QString relative() const;
    /**
     * @brief isNull
     * @return \c true if the FileName is null, \c false if it is valid.
     */
    bool isNull() const;
    bool operator==(const FileName &other) const;
    bool operator!=(const FileName &other) const;
    bool operator<(const FileName &other) const;
    /**
     * @brief exists
     * @return \c true if the file currently exists on the disk, \c false otherwise.
     */
    bool exists() const;

    /**
     * @brief Conversion to absolute local file url.
     */
    explicit operator QUrl() const;

private:
    // During previous profilation it showed that converting between absolute and relative took quite some time,
    // so to avoid that, I store both.
    QString m_relativePath;
    QString m_absoluteFilePath;
    bool m_isNull;
};

uint qHash(const DB::FileName &fileName);
typedef QSet<DB::FileName> FileNameSet;
}

Q_DECLARE_METATYPE(DB::FileName)

#endif // KPABASE_FILENAME_H
// vi:expandtab:tabstop=4 shiftwidth=4:
